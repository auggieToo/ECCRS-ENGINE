
#import "jotter-notes.typ": *

#let hs(s) = text(fill: rgb("#ff0600"), s)

= 3. Finite Markov Decision 

== 3.1 The agent's environment Interface .

The reinforcement learning problem is meant to be a straightforward framing
of the problem of learning from interaction to achieve a goal. The learner and
decision-maker is called the agent. The thing it interacts with, comprising
everything outside the agent, is called the environment. These interact con-
tinually, the agent selecting actions and the environment responding to those
actions and presenting new situations to the agent. The environment gives rises
rewards, that the agent tries to maximize.

The agent and the environment interact at each of a sequence of discrete time
steps, $t= 1, 2, dots$,. At each time step, the agent receives some   
representation of the environment's state $s_t in cal(S)$, where $cal(S)$ is 
a set of possible states, and on that Basis selects an action , $A_t in cal(A)(S_t)$
where $cal(A)(S_t)$ is a set of actions availible in set state $S_t$. One time 
step, in part as a consequence of its action,the agent receives a numerical *reward*, 
$R_(t + 1) in cal(R) subset RR$ and finds itself in a new different state.

At each time step, the agent implements a mapping from states to probabilities of 
selecting each  possible action.This mapping is called the agent's policy and 
is denoted as $pi_t$, where $pi_t (a | s)$ is the probability that $A_t = a$ if $S_t = s$
RL methods specifies how the agent changes its policy as a result of its experience. 
The agent’s goal, roughly speaking, is to maximize
the total amount of reward it receives over the long run.

What we chose as part of the environment is particulary up to us, but
The general rule we follow is that anything that cannot be changed ar-
bitrarily by the agent is considered to be outside of it and thus part of its
environment. We do not assume that everything in the environment is un-
known to the agent. For example, the agent often knows quite a bit about
how its rewards are computed as a function of its actions and the states in
which they are taken. But we always consider the reward computation to be
external to the agent because it deﬁnes the task facing the agent and thus
must be beyond its ability to change arbitrarily. In fact, in some cases the
agent may know everything about how its environment works and still face
a diﬃcult reinforcement learning task, just as we may know exactly how a
puzzle like Rubik’s cube works, but still be unable to solve it. The agent–
environment boundary represents the limit of the agent’s absolute control, not
of its knowledge. 

==  3.2 Goals and Rewards 

In RL , the goal or the purpose of the agent is formalized in term of reward 
signals, the agent's goal is to maximize the total amount of reward not the immediate 
reward. We can clearly state this informal idea as the  *reward hypothesis*:
#align(center)[

  That all of what we mean by goals and purposes can be well
  thought of as the maximization of the expected value of the cu-
  mulative sum of a received scalar signal (called reward).

]

If we want the agent to do something for us, we must reward the 
points in such a way that maximizing the rewards , the agent will 
achieve our goal.
It is thus critical that the rewards we set up truly
indicate what we want accomplished. In particular, the reward signal is not
the place to impart to the agent prior knowledge about how to achieve what we
want it to do.4 For example, a chess-playing agent should be rewarded only
for actually winning, not for achieving subgoals such taking its opponent’s
pieces or gaining control of the center of the board. If achieving these sorts
of subgoals were rewarded, then the agent might ﬁnd a way to achieve them
without achieving the real goal. For example, it might ﬁnd a way to take the
opponent’s pieces even at the cost of losing the game. The reward signal is
your way of communicating to the robot what you want it to achieve, not how
you want it achieved.

== 3.3 Returns 

How do we define the concept oif maximizing the rewards formally: 
If the sequence of rewards received after time step $t$ is denoted 
$R_(t+1) , R_(t+2) , R_(t+3), dots $ then what precise aspect of
this sequence do we wish to maximize? In general, we seek to maximize the
expected return, where the return $G_t$ is specified as some 
specific function of the reward sequence.In the simplest case, 
it is the sum of the rewards , 
$
G_t = R_(t+1) + R_(t+2) + R_(t+3)  + dots + R_T 
$
where T is the final time step. 
This approach makes sense in applications in
which there is a natural notion of ﬁnal time step, that is, when the agent 
environment interaction breaks naturally into subsequences, which we call
episodes, such as plays of a game, trips through a maze, or any sort of re-
peated interactions. Each episode ends in a special state called the terminal
state, followed by a reset to a standard starting state or to a sample from a
standard distribution of starting states. Tasks with episodes of this kind are
called episodic tasks. In episodic tasks we sometimes need to distinguish the
set of all nonterminal states, denoted $cal(S)$, from the set of all states plus the
terminal state, denoted $cal(S)^+$

On the other hand, in many cases the agent–environment interaction does
not break naturally into identiﬁable episodes, but goes on continually without
limit. For example, this would be the natural way to formulate a continual
process-control task, or an application to a robot with a long life span. We
call these continuing tasks. The sum return formulation is a bit 
problematic in this case as the final time step, $T = infinity$,and the 
return which we are in turn trying to maximize could be infinite


The additional concept that we need is that of discounting. According to
this approach, the agent tries to select actions so that the sum of the discounted
rewards it receives over the future is maximized.In particular it 
choses ,$A_t$ to maximize the expected *discounted return*: 

$
G_t = R_(t+1) + gamma  R_(t+2) + gamma^2 R_(t+3) + dots = sum_(k=0)^infinity gamma^k R_(t+k+1) 
$

where $gamma$ is a parameter $0 <= gamma <= 1$, called the discount rate. 

The discount rate determines the present value of future rewards: a reward
received k time steps in the future is worth only $gamma^(k-1)$ times it would be 
worth if it were received immediately.If $gamma < 1$, the infinite sum has a value as 
long as the sequence {$R_k$} is bounded. if $gamma = 0$ he agent
is “myopic” in being concerned only with maximizing immediate rewards: its
objective in this case is to learn how to choosei $A_t$ so as to maximize only $R_(t+1)$

If each of the agent’s actions happened to inﬂuence only the immediate
reward, not future rewards as well, then a myopic agent could maximize (3.2)
by separately maximizing each immediate reward. But in general, acting to
maximize immediate reward can reduce access to future rewards so that the
return may actually be reduced. As $γ$ approaches 1, the objective takes future
rewards into account more strongly: the agent becomes more farsighted.

== 3.4 Unified Notation for Episodic and continuing Tasks  

We establish a notation to speak of the agent-environment that breaks down into 
sequence of seperate episodes and one in which it does not. o be precise about 
episodic tasks requires some additional notation 

Let $S_(t,i)$ the state representation of at time $t$ in episode $i$, and 
similarly for  $A_(t,i), R_(t, i), pi_(t, i), T, e t c$
However,
it turns out that, when we discuss episodic tasks we will almost never have to
distinguish between diﬀerent episodes. We will almost always be considering
a particular single episode, or stating something that is true for all episodes.
So, in practice we can abuse the notation and drop the subscript $i$.

We need one other convention to obtain a single notation that covers both
episodic and continuing tasks. We have deﬁned the return as a sum over a ﬁnite
number of terms in one case (3.1) and as a sum over an inﬁnite number of terms
in the other (3.2). These can be uniﬁed by considering episode termination to
be the entering of a special absorbing state that transitions only to itself and
that generates only rewards of zero.

$
G_t = sum_(k=0)^(T-t-1) gamma^k R_(t+k+1)
$
We use these
conventions throughout the rest of the book to simplify notation and to express
the close parallels between episodic and continuing tasks.

== 3.5 Markov Property 

In the reinforcement learning framework, the agent makes its decisions as a
function of a signal from the environment called the environment’s state. In
this section we discuss what is required of the state signal, and what kind of
information we should and should not expect it to provide. In particular, we
formally deﬁne a property of environments and their state signals that is of
particular interest, called the Markov property.

The state is simply whatever information the agent has to act on, produced by some preprocessing 
step the book treats as part of the environment rather than something to be designed or learned 
, the focus is on choosing actions given the signal, not on building it. 
That signal can go well beyond raw sensation: it may be a processed form of 
current input, or something accumulated over time (a scene assembled across 
eye movements, an object remembered after looking away, the meaning of "yes" 
depending on the question that preceded it, velocity derived from two position 
readings). But it also shouldn't be expected to carry everything relevant, 
the next card in the deck, the caller's identity, an unconscious patient's 
internal injuries are all hidden from the agent because no sensation ever 
revealed them. The standard is memory, not omniscience: the agent isn't blamed 
for never having learned something, only for having received the information and 
then lost it.

What we would like, ideally, is a state signal that summarizes past sensa-
tions compactly, yet in such a way that all relevant information is retained.
This normally requires more than the immediate sensations, but never more
than the complete history of all past sensations. A state signal that suc-
ceeds in retaining all relevant information is said to be Markov, or to have
the Markov property (we deﬁne this formally below).e.g checkers position , the current 
config of the board is a markov property , and for a cannonbal, the current position 
and velocity is all that matter for its future flights, it does not matter how 
that current position or velocity came to be.  Sometimes called independence of paths. 

We now formally deﬁne the Markov property for the reinforcement learning
problem. To keep the mathematics simple, we assume here that there are a
ﬁnite number of states and reward values. This enables us to work in terms
of sums and probabilities rather than integrals and probability densities, but
the argument can easily be extended to include continuous states and rewards.
Consider how a general environment might respond at time t + 1 to the action
taken at time t. In the most general, causal case this response may depend
on everything that has happened earlier. In this case the dynamics can be
deﬁned only by specifying the complete probability distribution:

$ Pr {R_(t+1) = r, S_(t+1) = s' | S_0, A_o, R_1, dots , S_(t-1), A_(t-1) , R_t, S_t, A-t} $

for all $r, s'$ and possible values of the past events,$S_0, A_o, R_1, dots , S_(t-1), A_(t-1) , R_t, S_t, A-t$

If the state signal has the Markov property, on the other
hand, then the environment’s response at t + 1 depends only on the state and
action representations at t, in which case the environment’s dynamics can be
deﬁned by specifying only: 
$
p r (s',r | s, a) = Pr{R_(t+1) =r , S_(t+1)=s' | S_t, A_t}
$
n other words, a state signal has the Markov property,
and is a Markov state, if and only if (3.5) is equal to (3.4) for all s′ , r, and
histories $S_0, A_o, R_1, dots , S_(t-1), A_(t-1) , R_t, S_t, A-t$

In this case, the environment
and task as a whole are also said to have the Markov property.
we always want the state to be a good basis for predicting
future rewards and for selecting actions. In cases in which a model of the
environment is learned (see Chapter 8), we also want the state to be a good
basis for predicting subsequent states. Markov states provide an unsurpassed
basis for doing all of these things.

To the extent that the state approaches the
ability of Markov states in these ways, one will obtain better performance from
reinforcement learning systems. For all of these reasons, it is useful to think of
the state at each time step as an approximation to a Markov state, although
one should remember that it may not fully satisfy the Markov property.

The Markov property is important in reinforcement learning because 
decisions and values are assumed to be a function only of the current state.
