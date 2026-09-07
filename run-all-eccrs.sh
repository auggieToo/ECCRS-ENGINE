#!/usr/bin/env bash
# Sweep every rule set in tests-rules/ through the ECCRS stage.
# Output name is derived from the rules filename:
#   tests-rules/bcancer-rules-tot-ovr.txt -> results/bcancer-rules-tot-ovr_res.txt
#
# Usage:
#   ./run-all.sh                  # everything
#   ./run-all.sh bank bcancer     # only those datasets
#   DRY=1 ./run-all.sh            # print commands, run nothing

set -u

RULES_DIR=tests-rules
OUT_DIR=results
EXTRA_ARGS="--aco"

cd "$(dirname "$0")"
mkdir -p "$OUT_DIR"

# Which instance file(s) each dataset is run against.
instances_for() {
    case "$1" in
        bank)     echo "bank-inst.csv" ;;
        bcancer)  echo "bcancer-inst-csv.csv" ;;
        mushroom) echo "mushroom-inst.csv" ;;
        scholar)  echo "scholar-inst.txt scholar-inst1.txt scholar-inst2.txt scholar-inst3.txt" ;;
        *)        echo "" ;;
    esac
}

wanted=("$@")
want() {
    [ ${#wanted[@]} -eq 0 ] && return 0
    for w in "${wanted[@]}"; do [ "$w" = "$1" ] && return 0; done
    return 1
}

pass=0; fail=0; failed=()

for rules in "$RULES_DIR"/*-rules*.txt; do
    [ -e "$rules" ] || continue
    base=$(basename "$rules" .txt)          # bcancer-rules-tot-ovr
    dataset=${base%%-rules*}                # bcancer

    want "$dataset" || continue

    insts=$(instances_for "$dataset")
    if [ -z "$insts" ]; then
        echo "!! no instance mapping for '$dataset' (from $rules), skipping" >&2
        continue
    fi
    multi=0
    [ "$(echo "$insts" | wc -w)" -gt 1 ] && multi=1

    for inst in $insts; do
        instpath="$RULES_DIR/$inst"
        if [ ! -f "$instpath" ]; then
            echo "!! missing instance $instpath, skipping" >&2
            continue
        fi

        if [ "$multi" -eq 1 ]; then
            tag="${base}__$(basename "$inst" | sed 's/\.[^.]*$//')"
        else
            tag="$base"
        fi
        out="../$OUT_DIR/${tag}_res.txt"

        echo "=== $base  x  $inst -> $OUT_DIR/${tag}_res.txt"

        if [ "${DRY:-0}" = "1" ]; then
            echo "    make run-eccrs RULES=\"$rules\" INSTANCE=\"$instpath\" ECCRS_ARGS=\"$EXTRA_ARGS --out=$out\""
            continue
        fi

        # Force stage 1 to regenerate: make compares timestamps, not
        # variable values, so a changed RULES alone will not re-run gen.
        rm -f rules.c rules.h eccrs/*.o

        if make run-eccrs \
                RULES="$rules" \
                INSTANCE="$instpath" \
                ECCRS_ARGS="$EXTRA_ARGS --out=$out"; then
            pass=$((pass+1))
        else
            fail=$((fail+1))
            failed+=("$base x $inst")
        fi
        echo
    done
done

echo "--------------------------------------------"
echo "ok: $pass   failed: $fail"
for f in "${failed[@]:-}"; do [ -n "$f" ] && echo "  FAILED: $f"; done
[ "$fail" -eq 0 ]
