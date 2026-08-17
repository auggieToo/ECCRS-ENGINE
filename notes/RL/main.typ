
#import "jotter-notes.typ": *

#show: jotter-notes.with(
  title: [Reinforcement Learning],
  author: [August],
  date: datetime.today().display("[day] [month repr:short]"),
)
#set math.equation(numbering: n => numbering(
  "1.1", counter(heading).get().first(), n,
))
#show heading.where(level: 1): it => {
  counter(math.equation).update(0)
  it
}



#include "chp1.typ"
#include "ch2.typ"
#include "ch3.typ"
