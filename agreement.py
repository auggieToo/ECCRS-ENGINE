#!/usr/bin/env python3
import csv
import os
import sys

OUT = "out"
MSW_FILE = os.path.join(OUT, "eccrsResults.csv")
LC_FILE = os.path.join(OUT, "lexicogrResults.csv")
TABLE_FILE = os.path.join(OUT, "lexEccrsAgree.txt")
ROWS_FILE = os.path.join(OUT, "agreement_per_instance.csv")

OUTCOMES = ["agree", "both_abstain", "disagree",
            "lc_abstains", "msw_abstains", "conflict"]

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


def load(path):
    if not os.path.exists(path):
        sys.exit(f"error: file not found: {path}")
    with open(path, newline="", encoding="utf-8-sig") as f:
        reader = csv.DictReader(f)
        ids = [c for c in ("instance_id", "instanceid", "id") if c in reader.fieldnames]
        if not ids:
            sys.exit(f"error: {path}: no instance id column in {reader.fieldnames}")
        return {r[ids[0]].strip(): r for r in reader}


def msw_label(v):
    s = v.strip().lower()
    if s in ("1", "0"):
        return int(s)
    if s in ("", "abstain"):
        return None
    sys.exit(f"error: {MSW_FILE}: unrecognised prediction {v!r}")


def classify(msw, m, nm):
    if m and nm:
        return "conflict"
    lc_abstained = not m and not nm
    if msw is None:
        return "both_abstain" if lc_abstained else "msw_abstains"
    if lc_abstained:
        return "lc_abstains"
    return "agree" if (msw == 1 and m) or (msw == 0 and nm) else "disagree"


def main():
    name = sys.argv[1] if len(sys.argv) > 1 else "Dataset"

    msw = load(MSW_FILE)
    lc = load(LC_FILE)

    counts = dict.fromkeys(OUTCOMES, 0)
    rows = []
    for key in sorted(msw.keys() & lc.keys(), key=lambda k: (len(k), k)):
        lab = msw_label(msw[key]["prediction"])
        m = int(lc[key]["entail_m"])
        nm = int(lc[key]["entail_not_m"])
        outcome = classify(lab, m, nm)
        counts[outcome] += 1
        rows.append({"instance_id": key,
                     "msw_label": "abstain" if lab is None else lab,
                     "lc_entail_m": m, "lc_entail_not_m": nm,
                     "outcome": outcome})

    n = sum(counts.values())
    aligned = 100 * (counts["agree"] + counts["both_abstain"]) / n if n else 0.0
    cells = [name, str(n)] + [str(counts[o]) for o in OUTCOMES] + [f"{aligned:.2f}"]

    w = [max(len(h), len(c)) for h, c in zip(HDR, cells)]
    fmt = lambda xs: "  ".join(x.ljust(w[i]) for i, x in enumerate(xs)).rstrip()
    lines = [fmt(HDR), fmt(["-" * x for x in w]), fmt(cells), ""]
    lines += ["  " + l for l in LEGEND]

    only_msw = len(msw.keys() - lc.keys())
    only_lc = len(lc.keys() - msw.keys())
    if only_msw or only_lc:
        lines += ["", f"note: {only_lc} id(s) only in the LC file, "
                      f"{only_msw} only in the MSW file (not in N)"]

    with open(TABLE_FILE, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")

    with open(ROWS_FILE, "w", newline="", encoding="utf-8") as f:
        wr = csv.DictWriter(f, fieldnames=["instance_id", "msw_label",
                                           "lc_entail_m", "lc_entail_not_m",
                                           "outcome"])
        wr.writeheader()
        wr.writerows(rows)


if __name__ == "__main__":
    main()
