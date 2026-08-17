#import "jotter-notes.typ": *

#show: jotter-notes.with(
  title: [Asymptotics & Graphs],
  author: [Augii],
  date: datetime.today().display("[day] [month repr:short]"),
)

Rough idea for today: growth rates first, then how the pipeline fits together,
then the messy measurements from the lab.

$f(n) = O(g(n))$ means #circled[eventually] $f$ sits under some constant
multiple of $g$. The constant #pointer(note: [people forget this constantly])[doesn't]
matter, the tail does.

#graph(
  domain: (0.5, 6), width: 9.5cm, height: 6cm, grid: true,
  xlabel: [n], ylabel: [t],
  (x => x, [n]),
  (x => x * calc.log(x, base: 2), [n log n]),
  (x => 0.4 * x * x, [n²/2.5]),
)

Reading off the picture: quadratic loses #ar always, even with the friendly
constant. #scribble(style: "wave")[Constants shift curves, they never reorder
them.]

The compiler pipeline we sketched:

#flow[lex][parse][typecheck][emit]

with typecheck being the only stage that can reject. Data from the lab runs —
#correction[$n = 10^4$][$n = 10^5$, misread the log] elements, three
structures:

#stable(
  columns: (2, 1, 1, 1),
  [structure], [insert], [lookup], [notes],
  [sorted array], [$O(n)$], [$O(log n)$], [cache-friendly],
  [BST (unbalanced)], [$O(n)$ worst], [$O(n)$ worst], [degenerates on sorted
  input!],
  [hash table], [$O(1)$ am.], [$O(1)$ am.], [resize spikes],
)

#marginnote[the BST row is why we balance — see AVL notes]

#theorem(name: [Master, case 2])[
  If $T(n) = a T(n / b) + f(n)$ and $f(n) = Theta(n^(log_b a))$, then
  $T(n) = Theta(n^(log_b a) log n)$.
]

#proof[
  Each of the $log_b n$ levels contributes $Theta(n^(log_b a))$ work; sum
  over levels.
]

#sticky[
  Exam trick: mergesort is exactly case 2 with $a = b = 2$,
  $f(n) = Theta(n)$.
]

Measured scatter, one benchmark run — the outlier at the end is the resize
spike from the table above:

#graph(
  domain: (0, 10), range-y: (0, 60), width: 8.5cm, height: 5cm,
  points: ((1, 4), (2, 7), (3, 11), (4, 13), (5, 18), (6, 21), (7, 24),
    (8, 27), (9, 52)),
  point-label: [resize!],
  (x => 3 * x + 1, [3n+1 fit]),
)

#todo[re-run with reserve() and see if the spike disappears]

#warning[
  Amortized $O(1)$ is a statement about a _sequence_ of operations.
  #strike-hand[Every operation is fast] Any single one can be slow.
]

#photo(width: 6cm, caption: [board photo — recursion tree for case 2],
  graph(domain: (0, 4), width: 5.4cm, height: 3.6cm,
    (x => calc.pow(2, x), none)))
