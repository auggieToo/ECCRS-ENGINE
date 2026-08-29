#!/usr/bin/env python3
"""
ruleset_diff.py
---------------
Compare two ECCRS most-specific-wins (MSW) trace files produced over the
SAME instances by two different rule sets: one known-correct, one that
contains violations. Answers two questions:

  1. LABEL: does the violation change what the classifier predicts?
  2. SUPPORT: when the label survives, does the violation change WHY,
     i.e. which rules apply and which maximally specific rules win?

Both files use the trace schema:

    instance_id,applicable_rules,overrides,incl_max,prediction
    17754,"r19","r19 applies|","r19",0

Every instance present in both files falls into exactly one label
outcome, so the outcome columns sum to N:

    agree                both decisive, same label
    disagree             both decisive, different labels
    both abstain         neither rule set commits to a label
    correct abstains     only the correct rule set abstains
    violating abstains   only the violating rule set abstains

Support is reported on a second axis: of the instances where the label
survived, how many kept the identical applicable-rule set and the
identical incl_max set. A label that survives on different support is
still a divergence, and usually the more diagnostic one.

Outputs a console report, LaTeX tables, and two CSVs.
"""

from __future__ import annotations

import csv
import os
import re
from dataclasses import dataclass, field

# --------------------------------------------------------------------------
# CONFIGURATION -- one entry per (correct, violating) trace pair
# --------------------------------------------------------------------------

PAIRS = [
    {"name": "Mushroom",
     "violating":   "explanation_traces.csv",
     "correct": "bcancer_eccrs_pred_traces.csv"},
]

OUTPUT_DIR = "output"

COLS = {
    "id":               ["instance_id", "instanceid", "id"],
    "applicable_rules": ["applicable_rules", "applicable", "rules"],
    "overrides":        ["overrides", "override", "trace"],
    "incl_max":         ["incl_max", "inclmax", "maximal", "winners"],
    "label":            ["prediction", "pred", "label"],
}

MSW_ABSTAIN_TOKENS = {"", "abstain", "abstained", "none", "null", "na", "n/a",
                      "nan", "undecided", "unknown", "-"}
MSW_TRUE_TOKENS  = {"1", "true", "yes", "pos", "positive", "y", "t"}
MSW_FALSE_TOKENS = {"0", "false", "no", "neg", "negative", "n", "f"}

OUTCOMES = ["agree", "disagree", "both_abstain",
            "correct_abstains", "violating_abstains"]

SPLIT = re.compile(r"[,;|]+")

# --------------------------------------------------------------------------


@dataclass
class Result:
    """Divergence counts for a single trace pair."""
    name: str
    counts: dict = field(default_factory=lambda: {o: 0 for o in OUTCOMES})
    # 3x3 label confusion: (correct label, violating label), None = abstain
    cm: dict = field(default_factory=dict)
    # support drift, counted only where the label survived (agree/both_abstain)
    applicable_differs: int = 0
    incl_max_differs: int = 0
    jaccard_sum: float = 0.0
    jaccard_n: int = 0
    only_in_correct: int = 0
    only_in_violating: int = 0
    rows: list = field(default_factory=list)      # divergent instances only

    @property
    def matched(self) -> int:
        return sum(self.counts.values())

    def __getattr__(self, item):
        if item in OUTCOMES:
            return self.counts[item]
        raise AttributeError(item)

    @property
    def label_preserved(self) -> int:
        return self.counts["agree"] + self.counts["both_abstain"]

    @property
    def label_rate(self) -> float:
        return self.label_preserved / self.matched if self.matched else 0.0

    @property
    def intact(self) -> int:
        """Label preserved AND both rule sets identical."""
        return self.label_preserved - self.applicable_differs_or_incl_max

    @property
    def applicable_differs_or_incl_max(self) -> int:
        return self._support_differs

    @property
    def intact_rate(self) -> float:
        return self.intact / self.matched if self.matched else 0.0

    @property
    def mean_jaccard(self) -> float:
        return self.jaccard_sum / self.jaccard_n if self.jaccard_n else 1.0

    _support_differs: int = 0

    def add(self, other):
        for o in OUTCOMES:
            self.counts[o] += other.counts[o]
        for k, v in other.cm.items():
            self.cm[k] = self.cm.get(k, 0) + v
        self.applicable_differs += other.applicable_differs
        self.incl_max_differs += other.incl_max_differs
        self._support_differs += other._support_differs
        self.jaccard_sum += other.jaccard_sum
        self.jaccard_n += other.jaccard_n
        self.only_in_correct += other.only_in_correct
        self.only_in_violating += other.only_in_violating


# --------------------------------------------------------------------------
# I/O helpers
# --------------------------------------------------------------------------

def resolve(header, candidates, path, required=True):
    lower = {h.lower().strip(): h for h in header}
    for c in candidates:
        if c in lower:
            return lower[c]
    if not required:
        return None
    raise SystemExit(
        f"error: {path}\n  none of {candidates} found in columns {list(header)}"
    )


def read_csv(path):
    if not os.path.exists(path):
        raise SystemExit(f"error: file not found: {path}")
    with open(path, newline="", encoding="utf-8-sig") as f:
        reader = csv.DictReader(f)
        if reader.fieldnames is None:
            raise SystemExit(f"error: empty file: {path}")
        return list(reader), reader.fieldnames


def to_label(value, path, column, key):
    """Parse an MSW label into 1, 0, or None (the classifier abstained)."""
    s = str(value).strip().lower()
    if s in MSW_ABSTAIN_TOKENS:
        return None
    if s in MSW_TRUE_TOKENS:
        return 1
    if s in MSW_FALSE_TOKENS:
        return 0
    try:
        v = int(float(s))
    except ValueError:
        raise SystemExit(
            f"error: {path}: unrecognised label {value!r} "
            f"in column '{column}' at id {key}"
        )
    if v not in (0, 1):
        raise SystemExit(
            f"error: {path}: label must be 0 or 1, got {v} at id {key}"
        )
    return v


def rule_set(value) -> frozenset:
    """Parse a rule-set cell such as 'r19' or 'r3|r19' or 'r3, r19'."""
    if value is None:
        return frozenset()
    parts = (p.strip().strip("\"'") for p in SPLIT.split(str(value)))
    return frozenset(p for p in parts if p)


def fmt_set(s) -> str:
    return "|".join(sorted(s)) if s else "-"


def jaccard(x, y) -> float:
    if not x and not y:
        return 1.0
    return len(x & y) / len(x | y)


def show(label) -> str:
    return "abstain" if label is None else str(label)


# --------------------------------------------------------------------------
# Core comparison
# --------------------------------------------------------------------------

def classify(lc, lv):
    """lc / lv are the correct and violating labels; None means abstain."""
    if lc is None and lv is None:
        return "both_abstain"
    if lc is None:
        return "correct_abstains"
    if lv is None:
        return "violating_abstains"
    return "agree" if lc == lv else "disagree"


def load(path):
    rows, head = read_csv(path)
    cols = {
        "id":   resolve(head, COLS["id"], path),
        "app":  resolve(head, COLS["applicable_rules"], path),
        "ovr":  resolve(head, COLS["overrides"], path, required=False),
        "max":  resolve(head, COLS["incl_max"], path),
        "lab":  resolve(head, COLS["label"], path),
    }
    return {str(r[cols["id"]]).strip(): r for r in rows}, cols


def compare(name, path_correct, path_violating) -> Result:
    idx_c, col_c = load(path_correct)
    idx_v, col_v = load(path_violating)

    res = Result(name=name)
    res.only_in_correct   = len(set(idx_c) - set(idx_v))
    res.only_in_violating = len(set(idx_v) - set(idx_c))

    for key in sorted(set(idx_c) & set(idx_v), key=lambda k: (len(k), k)):
        rc, rv = idx_c[key], idx_v[key]

        lab_c = to_label(rc[col_c["lab"]], path_correct,   col_c["lab"], key)
        lab_v = to_label(rv[col_v["lab"]], path_violating, col_v["lab"], key)

        app_c, app_v = rule_set(rc[col_c["app"]]), rule_set(rv[col_v["app"]])
        max_c, max_v = rule_set(rc[col_c["max"]]), rule_set(rv[col_v["max"]])

        outcome = classify(lab_c, lab_v)
        res.counts[outcome] += 1
        res.cm[(lab_c, lab_v)] = res.cm.get((lab_c, lab_v), 0) + 1

        res.jaccard_sum += jaccard(app_c, app_v)
        res.jaccard_n += 1

        app_diff = app_c != app_v
        max_diff = max_c != max_v
        label_kept = outcome in ("agree", "both_abstain")

        if label_kept:
            if app_diff:
                res.applicable_differs += 1
            if max_diff:
                res.incl_max_differs += 1
            if app_diff or max_diff:
                res._support_differs += 1

        if not label_kept or app_diff or max_diff:
            res.rows.append({
                "dataset": name,
                "instance_id": key,
                "outcome": outcome,
                "label_correct": show(lab_c),
                "label_violating": show(lab_v),
                "applicable_correct": fmt_set(app_c),
                "applicable_violating": fmt_set(app_v),
                "applicable_only_correct": fmt_set(app_c - app_v),
                "applicable_only_violating": fmt_set(app_v - app_c),
                "incl_max_correct": fmt_set(max_c),
                "incl_max_violating": fmt_set(max_v),
                "support_differs": int(app_diff or max_diff),
                "jaccard_applicable": f"{jaccard(app_c, app_v):.4f}",
            })

    return res


# --------------------------------------------------------------------------
# Reporting
# --------------------------------------------------------------------------

HDR = ["Dataset", "N", "Agree", "Disagree", "Both abstain",
       "Correct abstains", "Violating abstains", "Label kept (%)"]

HDR2 = ["Dataset", "Label kept", "Support identical", "Applicable differs",
        "Incl-max differs", "Unchanged (%)", "Mean Jaccard"]

LEGEND = [
    "N                   instances present in both trace files (outcome columns sum to N)",
    "Agree               both rule sets decisive, same label",
    "Disagree            both rule sets decisive, different labels",
    "Both abstain        neither rule set commits to a label",
    "Correct abstains    only the correct rule set abstains",
    "Violating abstains  only the violating rule set abstains",
    "Label kept (%)      (Agree + Both abstain) / N",
]

LEGEND2 = [
    "Label kept          instances the violation did not relabel",
    "Support identical   of those, same applicable-rule set AND same incl_max",
    "Applicable differs  of those, the violation changed which rules apply",
    "Incl-max differs    of those, the violation changed which rules win",
    "Unchanged (%)       label kept on identical support, over all N",
    "Mean Jaccard        mean overlap of applicable-rule sets over all N",
]


def row_cells(r):
    return [r.name, f"{r.matched}", f"{r.agree}", f"{r.disagree}",
            f"{r.both_abstain}", f"{r.correct_abstains}",
            f"{r.violating_abstains}", f"{100 * r.label_rate:.2f}"]


def row_cells2(r):
    return [r.name, f"{r.label_preserved}", f"{r.intact}",
            f"{r.applicable_differs}", f"{r.incl_max_differs}",
            f"{100 * r.intact_rate:.2f}", f"{r.mean_jaccard:.4f}"]


def emit_table(hdr, rows):
    w = [max(len(hdr[i]), max(len(r[i]) for r in rows)) for i in range(len(hdr))]
    line = "  ".join("-" * x for x in w)

    def emit(cells):
        print("  ".join(c.ljust(w[i]) for i, c in enumerate(cells)))

    print()
    emit(hdr)
    print(line)
    for r in rows[:-1]:
        emit(r)
    print(line)
    emit(rows[-1])
    print()


def confusion(results, total):
    """3x3 label confusion, correct rule set down, violating across."""
    order = [1, 0, None]
    print("  label confusion (rows = correct rule set, cols = violating)")
    print()
    hdr = ["", "1", "0", "abstain"]
    rows = [hdr]
    for a in order:
        rows.append([show(a)] + [str(total.cm.get((a, b), 0)) for b in order])
    w = [max(len(r[i]) for r in rows) for i in range(4)]
    for i, r in enumerate(rows):
        print("    " + "  ".join(c.rjust(w[j]) for j, c in enumerate(r)))
        if i == 0:
            print("    " + "  ".join("-" * x for x in w))
    print()


def console_report(results):
    total = Result(name="Overall")
    for r in results:
        total.add(r)

    emit_table(HDR, [row_cells(r) for r in results] + [row_cells(total)])
    for l in LEGEND:
        print("  " + l)

    emit_table(HDR2, [row_cells2(r) for r in results] + [row_cells2(total)])
    for l in LEGEND2:
        print("  " + l)
    print()

    confusion(results, total)

    for r in results:
        if r.only_in_correct or r.only_in_violating:
            print(f"note: {r.name}: {r.only_in_correct} id(s) only in the "
                  f"correct trace, {r.only_in_violating} only in the violating "
                  f"trace (not comparable, so not in N)")
    return total


def latex_tables(results, total, path):
    def esc(s):
        return str(s).replace("_", r"\_").replace("&", r"\&").replace("%", r"\%")

    def line1(r):
        return (f"{esc(r.name)} & {r.matched} & {r.agree} & {r.disagree} & "
                f"{r.both_abstain} & {r.correct_abstains} & "
                f"{r.violating_abstains} & {100 * r.label_rate:.2f}" + r" \\")

    def line2(r):
        return (f"{esc(r.name)} & {r.label_preserved} & {r.intact} & "
                f"{r.applicable_differs} & {r.incl_max_differs} & "
                f"{100 * r.intact_rate:.2f} & {r.mean_jaccard:.4f}" + r" \\")

    L = [
        r"\begin{table}[t]",
        r"\centering",
        r"\caption{Effect of rule-set violations on MSW labels. Both trace "
        r"files cover the same instances; one is produced by the correct rule "
        r"set, the other by a rule set containing violations. Each instance "
        r"falls into exactly one outcome, so the outcome columns sum to $N$. "
        r"\emph{Agree}: both decisive with the same label. \emph{Disagree}: "
        r"both decisive with different labels. The remaining columns record "
        r"which of the two abstained. Label kept $=$ (Agree $+$ Both "
        r"abstain)$/N$.}",
        r"\label{tab:violation-labels}",
        r"\begin{tabular}{lrrrrrrr}",
        r"\toprule",
        r"Dataset & $N$ & Agree & Disagree & Both abstain & Correct abstains & "
        r"Violating abstains & Label kept (\%) \\",
        r"\midrule",
    ]
    L += [line1(r) for r in results]
    L += [r"\midrule", line1(total), r"\bottomrule", r"\end{tabular}",
          r"\end{table}", "", ]

    L += [
        r"\begin{table}[t]",
        r"\centering",
        r"\caption{Effect of rule-set violations on MSW support. Restricted to "
        r"the instances whose label the violation did not change: "
        r"\emph{support identical} keeps both the applicable-rule set and the "
        r"set of maximally specific winners, while the remaining columns count "
        r"instances that reach the same label through different rules. "
        r"Unchanged (\%) is taken over all $N$; Jaccard is the mean overlap of "
        r"applicable-rule sets, also over all $N$.}",
        r"\label{tab:violation-support}",
        r"\begin{tabular}{lrrrrrr}",
        r"\toprule",
        r"Dataset & Label kept & Support identical & Applicable differs & "
        r"Incl-max differs & Unchanged (\%) & Mean Jaccard \\",
        r"\midrule",
    ]
    L += [line2(r) for r in results]
    L += [r"\midrule", line2(total), r"\bottomrule", r"\end{tabular}",
          r"\end{table}"]

    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(L) + "\n")


def summary_csv(results, total, path):
    with open(path, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["dataset", "n", "agree", "disagree", "both_abstain",
                    "correct_abstains", "violating_abstains",
                    "label_kept_rate", "label_kept", "support_identical",
                    "applicable_differs", "incl_max_differs",
                    "unchanged_rate", "mean_jaccard_applicable",
                    "only_in_correct", "only_in_violating"])
        for r in list(results) + [total]:
            w.writerow([r.name, r.matched, r.agree, r.disagree, r.both_abstain,
                        r.correct_abstains, r.violating_abstains,
                        f"{r.label_rate:.6f}", r.label_preserved, r.intact,
                        r.applicable_differs, r.incl_max_differs,
                        f"{r.intact_rate:.6f}", f"{r.mean_jaccard:.6f}",
                        r.only_in_correct, r.only_in_violating])


FIELDS = ["dataset", "instance_id", "outcome", "label_correct",
          "label_violating", "applicable_correct", "applicable_violating",
          "applicable_only_correct", "applicable_only_violating",
          "incl_max_correct", "incl_max_violating", "support_differs",
          "jaccard_applicable"]


def divergence_csv(results, path):
    with open(path, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=FIELDS)
        w.writeheader()
        for r in results:
            w.writerows(r.rows)


# --------------------------------------------------------------------------

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    results = [compare(p["name"], p["correct"], p["violating"]) for p in PAIRS]
    total = console_report(results)

    tex = os.path.join(OUTPUT_DIR, "ruleset_diff_tables.tex")
    sm  = os.path.join(OUTPUT_DIR, "ruleset_diff_summary.csv")
    dv  = os.path.join(OUTPUT_DIR, "ruleset_diff_divergences.csv")

    latex_tables(results, total, tex)
    summary_csv(results, total, sm)
    divergence_csv(results, dv)

    print(f"wrote {tex}")
    print(f"wrote {sm}")
    print(f"wrote {dv}  ({sum(len(r.rows) for r in results)} divergent instance(s))")


if __name__ == "__main__":
    main()
