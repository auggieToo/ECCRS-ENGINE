#!/usr/bin/env python3
"""
agreement.py
------------
Compare Lexicographic Closure (LC) entailment against the ECCRS
most-specific-wins (MSW) classifier across one or more benchmark
datasets.

LC answers the queries F |~ m and F |~ not-m; MSW assigns the label
1 or 0, or abstains. Every instance present in both files falls into
exactly one outcome, so the outcome columns always sum to N:

    agree          both decisive and assign the same label
                     (MSW=1 with m entailed, or MSW=0 with not-m entailed)
    disagree       both decisive but assign different labels
    both abstain   MSW abstains and LC entails neither m nor not-m
    LC abstains    MSW is decisive, LC entails neither m nor not-m
    MSW abstains   MSW abstains, LC is decisive
    conflict       LC entails BOTH m and not-m (inconsistent KB)

Aligned (%) = (agree + both abstain) / N -- the two ways LC and MSW
can say the same thing.

Outputs a console table, a LaTeX booktabs table, and two CSVs.
"""

from __future__ import annotations

import csv
import os
from dataclasses import dataclass, field

# --------------------------------------------------------------------------
# CONFIGURATION -- add one entry per benchmark dataset
# --------------------------------------------------------------------------

DATASETS = [
    {"name": "Bank",     "lc": "bank_lexCl_results.csv",     "msw": "bank_eccrs_pred_traces.csv"},
    {"name": "Cancer",   "lc": "bcancer_lexCl_results.csv",  "msw": "bcancer_eccrs_pred_traces.csv"},
    {"name": "Mushroom", "lc": "results.csv",                "msw": "explanation_traces.csv"},
]

OUTPUT_DIR = "output"

# Column-name candidates, tried in order. Extend if your files differ.
COLS = {
    "lc_id":           ["instanceid", "instance_id", "queryid", "query_id", "id"],
    "lc_entail_m":     ["entail_m"],
    "lc_entail_not_m": ["entail_not_m", "entail_notm"],
    "msw_id":          ["instance_id", "instanceid", "id"],
    "msw_label":       ["prediction", "pred", "label"],
}

# Values in the MSW label column meaning the classifier abstained.
MSW_ABSTAIN_TOKENS = {"", "abstain", "abstained", "none", "null", "na", "n/a",
                      "nan", "undecided", "unknown", "-"}
MSW_TRUE_TOKENS  = {"1", "true", "yes", "pos", "positive", "y", "t"}
MSW_FALSE_TOKENS = {"0", "false", "no", "neg", "negative", "n", "f"}

OUTCOMES = ["agree", "both_abstain", "disagree", "lc_abstains",
            "msw_abstains", "conflict"]

# --------------------------------------------------------------------------


@dataclass
class Result:
    """Outcome counts for a single dataset."""
    name: str
    counts: dict = field(default_factory=lambda: {o: 0 for o in OUTCOMES})
    # 2x2 over rows where BOTH are decisive: (MSW label, LC label)
    cm: dict = field(default_factory=lambda: {(1, 1): 0, (1, 0): 0,
                                              (0, 1): 0, (0, 0): 0})
    only_in_lc: int = 0
    only_in_msw: int = 0
    rows: list = field(default_factory=list)

    @property
    def matched(self) -> int:
        return sum(self.counts.values())

    def __getattr__(self, item):
        # r.agree, r.disagree, r.conflict, ... read straight off counts
        if item in OUTCOMES:
            return self.counts[item]
        raise AttributeError(item)

    @property
    def aligned(self) -> int:
        return self.counts["agree"] + self.counts["both_abstain"]

    @property
    def aligned_rate(self) -> float:
        return self.aligned / self.matched if self.matched else 0.0

    @property
    def decisive_rate(self) -> float:
        """Agreement restricted to rows where both LC and MSW are decisive."""
        d = self.counts["agree"] + self.counts["disagree"]
        return self.counts["agree"] / d if d else 0.0

    def label_rate(self, label: int) -> float:
        """Of decisive rows where MSW assigned `label`, fraction LC confirms."""
        n = self.cm[(label, 1)] + self.cm[(label, 0)]
        return (self.cm[(label, label)] / n) if n else 0.0

    @property
    def kappa(self) -> float:
        """Cohen's kappa over rows where both LC and MSW are decisive."""
        n11, n10 = self.cm[(1, 1)], self.cm[(1, 0)]
        n01, n00 = self.cm[(0, 1)], self.cm[(0, 0)]
        n = n11 + n10 + n01 + n00
        if n == 0:
            return float("nan")
        po = (n11 + n00) / n
        pe = ((n11 + n10) * (n11 + n01) + (n01 + n00) * (n10 + n00)) / (n * n)
        return (po - pe) / (1 - pe) if pe != 1 else float("nan")

    def add(self, other):
        for o in OUTCOMES:
            self.counts[o] += other.counts[o]
        for k in self.cm:
            self.cm[k] += other.cm[k]


# --------------------------------------------------------------------------
# I/O helpers
# --------------------------------------------------------------------------

def resolve(header, candidates, path):
    """Find the first candidate column present in header (case-insensitive)."""
    lower = {h.lower().strip(): h for h in header}
    for c in candidates:
        if c in lower:
            return lower[c]
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


def to_int(value, path, column, key):
    try:
        return int(str(value).strip())
    except (ValueError, AttributeError):
        raise SystemExit(
            f"error: {path}: non-integer value {value!r} "
            f"in column '{column}' at id {key}"
        )


def to_label(value, path, column, key):
    """Parse an MSW label into 1, 0, or None (MSW abstained)."""
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


# --------------------------------------------------------------------------
# Core comparison
# --------------------------------------------------------------------------

def classify(msw, m, nm):
    """msw is 1, 0 or None (abstained); m/nm are the LC entailment flags."""
    if m == 1 and nm == 1:
        return "conflict"
    lc_abstained = (m == 0 and nm == 0)
    if msw is None:
        return "both_abstain" if lc_abstained else "msw_abstains"
    if lc_abstained:
        return "lc_abstains"
    return "agree" if ((msw == 1 and m == 1) or (msw == 0 and nm == 1)) \
        else "disagree"


def compare(name, path_lc, path_msw) -> Result:
    rows_lc, head_lc = read_csv(path_lc)
    rows_msw, head_msw = read_csv(path_msw)

    lc_id  = resolve(head_lc, COLS["lc_id"], path_lc)
    lc_m   = resolve(head_lc, COLS["lc_entail_m"], path_lc)
    lc_nm  = resolve(head_lc, COLS["lc_entail_not_m"], path_lc)
    msw_id = resolve(head_msw, COLS["msw_id"], path_msw)
    msw_l  = resolve(head_msw, COLS["msw_label"], path_msw)

    index_lc  = {str(r[lc_id]).strip(): r for r in rows_lc}
    index_msw = {str(r[msw_id]).strip(): r for r in rows_msw}

    res = Result(name=name)
    res.only_in_lc  = len(set(index_lc) - set(index_msw))
    res.only_in_msw = len(set(index_msw) - set(index_lc))

    for key in sorted(set(index_lc) & set(index_msw), key=lambda k: (len(k), k)):
        r_lc, r_msw = index_lc[key], index_msw[key]

        m   = to_int(r_lc[lc_m],  path_lc, lc_m,  key)
        nm  = to_int(r_lc[lc_nm], path_lc, lc_nm, key)
        msw = to_label(r_msw[msw_l], path_msw, msw_l, key)

        outcome = classify(msw, m, nm)
        res.counts[outcome] += 1
        if outcome in ("agree", "disagree"):
            res.cm[(msw, m)] += 1   # LC label: m=1 means m entailed

        res.rows.append(
            {"dataset": name, "instance_id": key,
             "msw_label": "abstain" if msw is None else msw,
             "lc_entail_m": m, "lc_entail_not_m": nm, "outcome": outcome}
        )

    return res


# --------------------------------------------------------------------------
# Reporting
# --------------------------------------------------------------------------

HDR = ["Dataset", "N", "Agree", "Both abstain", "Disagree",
       "LC abstains", "MSW abstains", "Conflict", "Aligned (%)"]

LEGEND = [
    "N             instances present in both files (the six outcome columns sum to N)",
    "Agree         both decisive, same label: MSW=1 with m entailed, or MSW=0 with not-m entailed",
    "Both abstain  MSW abstains and LC entails neither m nor not-m",
    "Disagree      both decisive, different labels",
    "LC abstains   MSW is decisive, LC entails neither m nor not-m",
    "MSW abstains  MSW abstains, LC is decisive",
    "Conflict      LC entails both m and not-m (inconsistent; should not occur)",
    "Aligned (%)   (Agree + Both abstain) / N",
]


def row_cells(r):
    return [r.name, f"{r.matched}", f"{r.agree}", f"{r.both_abstain}",
            f"{r.disagree}", f"{r.lc_abstains}", f"{r.msw_abstains}",
            f"{r.conflict}", f"{100 * r.aligned_rate:.2f}"]


def console_table(results):
    total = Result(name="Overall")
    for r in results:
        total.add(r)

    rows = [row_cells(r) for r in results] + [row_cells(total)]
    w = [max(len(HDR[i]), max(len(r[i]) for r in rows)) for i in range(len(HDR))]
    line = "  ".join("-" * x for x in w)

    def emit(cells):
        print("  ".join(c.ljust(w[i]) for i, c in enumerate(cells)))

    print()
    emit(HDR)
    print(line)
    for r in rows[:-1]:
        emit(r)
    print(line)
    emit(rows[-1])
    print()
    for l in LEGEND:
        print("  " + l)
    print()

    for r in results:
        if r.only_in_lc or r.only_in_msw:
            print(f"note: {r.name}: {r.only_in_lc} id(s) only in the LC file, "
                  f"{r.only_in_msw} only in the MSW file (not comparable, "
                  f"so not in N)")
    return total


def latex_table(results, total, path):
    def esc(s):
        return str(s).replace("_", r"\_").replace("&", r"\&").replace("%", r"\%")

    def line(r):
        return (f"{esc(r.name)} & {r.matched} & {r.agree} & {r.both_abstain} & "
                f"{r.disagree} & {r.lc_abstains} & {r.msw_abstains} & "
                f"{r.conflict} & {100 * r.aligned_rate:.2f}" + r" \\")

    L = [
        r"\begin{table}[t]",
        r"\centering",
        r"\caption{Agreement between Lexicographic Closure (LC) entailment and "
        r"the ECCRS most-specific-wins (MSW) classifier. Each instance falls "
        r"into exactly one outcome, so the six outcome columns sum to $N$. "
        r"\emph{Agree}: both are decisive and assign the same label, i.e.\ "
        r"$\mathrm{MSW}=1$ with $m$ entailed or $\mathrm{MSW}=0$ with $\neg m$ "
        r"entailed. \emph{Both abstain}: MSW abstains and LC entails neither "
        r"$m$ nor $\neg m$. \emph{Disagree}: both decisive, different labels. "
        r"\emph{LC abstains} and \emph{MSW abstains}: exactly one of the two "
        r"abstains. \emph{Conflict}: LC entails both $m$ and $\neg m$. "
        r"Aligned $=$ (Agree $+$ Both abstain)$/N$.}",
        r"\label{tab:agreement}",
        r"\begin{tabular}{lrrrrrrrr}",
        r"\toprule",
        r" & & \multicolumn{2}{c}{Same answer} & "
        r"\multicolumn{4}{c}{Different answer} & \\",
        r"\cmidrule(lr){3-4}\cmidrule(lr){5-8}",
        r"Dataset & $N$ & Agree & Both abstain & Disagree & LC abstains & "
        r"MSW abstains & Conflict & Aligned (\%) \\",
        r"\midrule",
    ]
    L += [line(r) for r in results]
    L += [
        r"\midrule",
        line(total),
        r"\bottomrule",
        r"\end{tabular}",
        r"\end{table}",
    ]
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(L) + "\n")


def summary_csv(results, total, path):
    with open(path, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["dataset", "n", "agree", "both_abstain", "disagree",
                    "lc_abstains", "msw_abstains", "conflict",
                    "aligned_rate", "agreement_rate_decisive_only",
                    "agreement_rate_msw1", "agreement_rate_msw0",
                    "cohens_kappa", "only_in_lc", "only_in_msw"])
        for r in list(results) + [total]:
            k = r.kappa
            w.writerow([r.name, r.matched, r.agree, r.both_abstain, r.disagree,
                        r.lc_abstains, r.msw_abstains, r.conflict,
                        f"{r.aligned_rate:.6f}", f"{r.decisive_rate:.6f}",
                        f"{r.label_rate(1):.6f}", f"{r.label_rate(0):.6f}",
                        "" if k != k else f"{k:.6f}",
                        r.only_in_lc, r.only_in_msw])


def per_instance_csv(results, path):
    with open(path, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=["dataset", "instance_id", "msw_label",
                                          "lc_entail_m", "lc_entail_not_m",
                                          "outcome"])
        w.writeheader()
        for r in results:
            w.writerows(r.rows)


# --------------------------------------------------------------------------

def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    results = [compare(d["name"], d["lc"], d["msw"]) for d in DATASETS]
    total = console_table(results)

    tex = os.path.join(OUTPUT_DIR, "agreement_table.tex")
    sm  = os.path.join(OUTPUT_DIR, "agreement_summary.csv")
    pi  = os.path.join(OUTPUT_DIR, "agreement_per_instance.csv")

    latex_table(results, total, tex)
    summary_csv(results, total, sm)
    per_instance_csv(results, pi)

    print(f"wrote {tex}")
    print(f"wrote {sm}")
    print(f"wrote {pi}")


if __name__ == "__main__":
    main()
