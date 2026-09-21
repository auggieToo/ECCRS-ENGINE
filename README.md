# EDRS:  Explanations using Defeasible Rule Sets

A workbench that checks the decisions of an **Exception-Closed Conjunctive Rule
Set (ECCRS)** classifier against **KLM-style defeasible entailment**. It
translates an exported ECCRS rule set into a KLM knowledge base, computes
**Lexicographic Closure** entailment over it, and compares the result, instance
by instance, against the classifier's own **Most-Specific-Wins (MSW)**
prediction. Explanation traces are produced on both sides.

The MSW–KLM correspondence theorem this workbench verifies is due to Madzime and
Meyer. This project is an independent C implementation of the translation and
entailment, an empirical check of the correspondence on benchmark rule sets, and
a study of how the correspondence breaks when the theorem's structural
assumptions are violated.

## Pipeline

The project runs in three stages, orchestrated by the top-level `Makefile`:

1. **eccrs** — parses a rule set (`rules.txt`) and an instance set, generates
   `rules.c` / `rules.h`, and builds the ECCRS classifier. Runs MSW inference,
   validates the alignment-theorem assumptions (sanity, strict global exception
   closure, no total override), and emits explanation traces.
2. **translate** — maps the exported ECCRS rule set into a KLM defeasible
   knowledge base (label atom `m` for class 1, `¬m` for class 0), producing the
   blob the reasoner consumes.
3. **klm** — computes BaseRank, SubsetRank, and Lexicographic Closure over the
   knowledge base; for each instance it queries `F |~ m` and `F |~ ¬m`, records
   agreement / conflict / abstention, and writes a results CSV.

## Requirements

- A C compiler (`gcc`) and `make`.
- **No external solver needed.** PicoSAT is vendored in `klm/vendor/` and
  compiled as part of the build.
- Python 3 for the comparison step (`agreement.py`); standard library only.

## Build

```sh
make            # builds all three stages
```

Or a single stage:

```sh
make eccrs
make translate
make klm
```

## Run

```sh
make run        # runs eccrs -> translate -> klm in order
```

`make run` executes the full pipeline: the classifier runs, its rule set is
translated, and the reasoner produces the agreement results. Individual stages
can be run with `make run-eccrs`, `make run-translate`, `make run-klm`.

## Inputs

Set on the `make` command line (defaults shown):

```sh
make run RULES=default-rules/rules.txt INSTANCE=default-rules/inst.txt
```

- `RULES` — the ECCRS rule set (numbered `IF ... THEN <label>` rules).
- `INSTANCE` — the set of complete feature patterns to classify.

## Comparing MSW against LC

The C pipeline produces the two sides' verdicts separately. `agreement.py`
compares them for one dataset:

```sh
python agreement.py [dataset-name]
```

It reads:

- `out/eccrsResults.csv` — `instance_id, prediction` (1 / 0 / abstain), the MSW side.
- `out/lexicogrResults.csv` — `instance_id, entail_m, entail_not_m`, the LC side.

and writes:

- `out/lexEccrsAgree.txt` — the agreement table with a legend of the outcome
  categories (Agree, Both abstain, Disagree, LC abstains, MSW abstains,
  Conflict).
- `out/agreement_per_instance.csv` — the per-instance breakdown.

Run the C pipeline first (`make run`) so both input CSVs exist, then run
`agreement.py`.

## Outputs

The `eccrs -> translate -> klm` pipeline writes to `out/`:

- `eccrsResults.csv` — MSW prediction per instance (1 / 0 / abstain).
- `eccrsTraces.txt` — ECCRS-side explanation traces.
- `assumptionCheck.txt` — alignment-theorem assumption validation report.
- `knowledgeBase.txt` — the translated KLM knowledge base.
- `baseRank.txt` — the BaseRank ranking.
- `lexicogrResults.csv` — LC entailment per instance (`entail_m`, `entail_not_m`).

`agreement.py` then adds:

- `lexEccrsAgree.txt` — agreement table and legend.
- `agreement_per_instance.csv` — per-instance comparison.

## Layout

```
eccrs/         ECCRS classifier: MSW inference, assumption validation, traces
translator/    ECCRS rule set -> KLM knowledge base
klm/           KLM reasoner: BaseRank, SubsetRank, Lexicographic Closure
  vendor/      vendored PicoSAT (picosat.c, picosat.h, config.h)
build/         object files
default-rules/ sample rule and instance sets
Makefile       top-level pipeline
```

## Clean

```sh
make clean
```

## Author

Augustine Mochoeneng. Supervised by Prof. Thomas Meyer, University of Cape Town.
