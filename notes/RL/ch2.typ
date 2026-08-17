
#import "jotter-notes.typ": *

#let hs(s) = text(fill: rgb("#ff0600"), s)

= 2. Multi-arm Bandits 

- The most important distinguishing feature of RL  from other 
  types of learning is that it uses information that #hs[evaluates]
  the actions taken rather than #hs[instruct] by giving correct decisions. 
- purely evaluative feedback indicates how good the action taken 
  is but not whether it is the best of worst action possible.
  Evaluative feedback is the basis of methods for function optimization, including evolutionary
  methods. 
- Purely instructive feedback, on the other hand, indicates the 
  correct action to take, independently of the action actually taken. This kind
  of feedback is the basis of supervised learning, which includes large parts of
  pattern classiﬁcation, artiﬁcial neural networks, and system identiﬁcation
- These two kinds of feedback seems to be quite distinct: evaluative feed 
  depends entire on the action taken, whereas instructive feedback 
  is independent of the action. 
- A *nonassociative* setting is one in which prior work involving 
  evaluative feedback has been done and it avoids much of the complexity
  of the full reinforcement learning problem.

== 2.1 AN $n$-armed Bandit problem

Consider the following learning problem. You are faced repeatedly with a
choice among n diﬀerent options, or actions. After each choice you receive a
numerical reward chosen from a stationary probability distribution that de-
pends on the action you selected. Your objective is to maximize the expected
total reward over some time period, for example, over 1000 action selections,
or time steps.

Each action has an expected or mean reward given that that action 
is selected;we call this the *value* of that action.If you knew the 
value of each action then it would be very trivial to solve the 
Problem: always select the action with the highest value. Thus, we assume 
you do not know the action values with certainity but you are free 
to make estimates.

If you maintain estimates of the action values, then at any time step there
is at least one action whose estimated value is greatest. We call this a greedy
action. If you select a greedy action, we say that you are exploiting your
current knowledge of the values of the actions. If instead you select one of
the nongreedy actions, then we say you are exploring, because this enables
you to improve your estimate of the nongreedy action’s value. Exploitation is
the right thing to do to maximize the expected reward on the one step, but
exploration may produce the greater total reward in the long run.





== 2.2 Action-value methods 
We begin by looking more closely at some simple methods for estimating the
values of actions and for using the estimates to make action selection decisions.

We denote the true (actual) value of action $a$ as $q(a)$ and the 
estimated value of on the $t$-th time step as $Q_t (a)$

Recall that the true value of an
action is the mean reward received when that action is selected. 

One natural way of estimating this is by averaging the rewards 
when the action was selected. In other words, if by the $t$-th 
time step action $a$ has been chosen $N_t (a)$ times prior to $t$, 
yielding the rewards $R_1 , R_2, dots ,R_(N_t (a)) $ then its value is 
estimated to be: 
\
$ Q_t (a) = (R_1 + R_2 + dots + R_(N_t (a)) ) / (N_t (a)) $

if $N_t (a) = 0$ then we define $Q_t (a)$ instead as some default 
value, such as $Q_1 (a) = 0$.

As $N_t (a) -> infinity $,  $Q_t (a)$  converges to $q(a)$. We call 
this the *sample-average* method for estimating action-values. 

Of course this is just one way to estimate the action value 
but not the best.
The simplest action rule is to select the action with the 
highest estimated action value,that is, to select at time $t$ one 
of the greedy actions , $A_i^*$ for which $Q_t (A_i^*) = max_a Q_t (A)$.
This greedy action selection can method can be written as: 
$
  A_t = "arg"max_a Q_t (a) 
$
where $"arg"max_a$ denotes the value of a at which the expression that follows
is maximized (with ties broken arbitrarily).
The greedy action  selection method always exploits the current 
knowledge to maximise immediate reward;it spends no time at sampling 
apparentely inferior actions to see if they might really be better. 
A simple alternative is to behave greedily most of the time but once 
to in a while behave, say with probability of $epsilon$, instead
to select randomly amongst all the actions with equal probability
independent of the value estimates.
We call methods using the near-greedy action selection rule as 
*$epsilon$-greedy* methods.

An advantage of these methods is that, in the limit as the
number of plays increases, every action will be sampled an inﬁnite number
of times, guaranteeing that $N_t (a) -> infinity$ for all $a$ and thus
ensuring that all $Q_t (a)$ converge to $q (a)$.This  implies 
that the probability of selecting the optimal action converges 
to greater than $1 - epsilon$

Another thing to note is what to do when the multiple actions 
have the same maximal action values. Such ties can be broken 
randomly. An alternative that has the same effect is to add 
a very small amount of randomness to each of the initial actions 
values , so that ties effectively never happen.

Finaly, The advantage of ε-greedy over greedy methods depends on the task. For
example, suppose the reward variance had been larger, say 10 instead of 1.
With noisier rewards it takes more exploration to ﬁnd the optimal action, and
ε-greedy methods should fare even better relative to the greedy method. On
the other hand, if the reward variances were zero, then the greedy method
would know the true value of each action after trying it once. In this case the
greedy method might actually perform best because it would soon ﬁnd the
optimal action and then never explore. But even in the deterministic case,
there is a large advantage to exploring if we weaken some of the other as-
sumptions. For example, suppose the bandit task were nonstationary, that is,
that the true values of the actions changed over time. In this case exploration
is needed even in the deterministic case to make sure one of the nongreedy
actions has not changed to become better than the greedy one.

== 2.3 Incremental Implementation 

THE Action-value method that was discussed above, estimate actions 
value by averaging the observed rewards. Hence, the obvious Implementation
is to maintain, for each action a , a record of all the rewards 
that have followed the selection of that action. Then the estimated 
of the value of action $a$ at time $t$ can be computed as:
$ Q_t (a) = (R_1 + R_2 + dots + R_(N_t (a)) ) / (N_t (a)) $
A problem with this strategy is that its memory and computational 
requirements grow over time without bound. 

It is easy to devise incremental update formulas for computing averages with small, constant com-
putation required to process each new reward.

For some action $a$,let $Q_k$ denote the estimate for its $k$th 
reward, that is the average of the first $k - 1 $ rewards.Given this 
average and the kth reward for the action, the average of the all 
the k-rewards can be computed by 
$
Q_(k + 1) &= 1/k sum_(i=1)^k R_i\
          &= 1/k (R_k + sum_(i=1)^(k - 1 ) R_i)\
          &= 1/k (R_k + (k - 1)Q_k + Q_k - Q_k)\
          &= 1/k (R_k + k Q_k - Q_k)\
          &=  Q_k  + 1/k (R_k- Q_k)\

$
This implementation requires memory for $Q_k$ and $k$. 

This is a update rule and has the form: 
$
"NewEstimate" <- "OldEstimate" + "stepSize" ["Target" - "OldEstimate"]

$

The expression $["Target" - "OldEstimate"]$ is the error in the 
estimate.It is
reduced by taking a step toward the “Target.” The target is presumed to
ndicate a desirable direction in which to move, though it may be noisy. In
the case above, for example, the target is the kth reward.

The step size parameter  used in the incremental method 
described above changes from time step to time step. 
We denote the step-size parameter as $alpha$ or more generallyb as 
$alpha_t(a)$, in this case, $alpha = 1 / k$

== 2.4 Tracking a nonstationary Problem.
The averaging method discussed so far are appropriate in a stationary 
environment , but not if the bandit's actual value is changing over time. 
In such cases, it makes sense to weight in recent rewards more heavily
than long past ones. One of the most popular ways of doing this is to use a
constant step-size parameter.For example, 
$
  Q_(k+1) = Q_k + alpha[R_k - Q_k]

$
where the step size parameter $alpha in (0,1]$ is constant.
This results in $Q_(k+1)$ being the weighted average of the past rewards 
and the initial estimate. 

$
  Q_K &= Q_k + alpha [R_k - Q_k]\
      &= alpha R_k + (1 - alpha)Q_k \
      &= alpha R_k + (1 - alpha)[alpha R_(k-1) + (1 - alpha)Q_(k-1)]\ 
      &= alpha R_k + (1 - alpha)alpha R_(k-1)  (1 - alpha)^2Q_(k-1)\
      &= alpha R_k + (1 - alpha)alpha R_(k-1) +  (1 - alpha)^2 alpha R_(k-2) + 
         dots + (1 - alpha)^(k-1) alpha R_1  + (1 - alpha)^k Q_1  \

     &= (1-alpha)^k Q_1 + sum_(i=1)^k alpha (1 - alpha)^(k-i)R_i 
$
This called the weighted average because the sum of the weights 
$(1 - alpha)^k + sum_(i=1)^k alpha (1 - alpha)^(k-i) = 1$. 
The weight given to $R_i$ decreases as the number of intervening rewards 
increases. In fact, the weight decreases exponential as the according to the 
exponent on $1-alpha$, (if $1-alpha = 0$ then all the weight goes on 
the very last reward,$R_k$ because of the convention that $0^0=1$).
This assumption is called *exponential, recency-weighted average*

Sometimes it is convenient to vary the step size parameter from step to step. 
Let $alpha_k (t)$ denote the step-size parameter used to process the reward 
received after the $k$th reward selection of $a$. As notes , 
 $alpha_k (t) = 1 / k$  results in the sample-average method, which is 
guranteed to connverge to the true action values by the law of large numbers.

But of course convergence is not guaranteed for all choices of the sequence
$ {alpha_k (a)}$.A well-
known result in stochastic approximation theory gives us the conditions re-
quired to assure convergence with probability 1:
$
  sum_(k=1)^infinity alpha_k(a) = infinity " and " sum_(k=1)^infinity alpha_k^2(a) < infinity  

$
The ﬁrst condition is required to guarantee that the steps are large enough to
eventually overcome any initial conditions or random ﬂuctuations. The second
condition guarantees that eventually the steps become small enough to assure
convergence.

Note that both conditions are met when $alpha_k(a) = 1/k$ but not for the constant 
parameter  $alpha_k(a) = alpha$. In the latter case, the second condition is not 
met indicating that the estimates never completely converge but continue 
to vary in response to the most recently received reward.


As we mentioned above, this is actually desirable in
a nonstationary environment, and problems that are eﬀectively nonstationary
are the norm in reinforcement learning.

Sequences that meet the two conditions often converge very slowly
or need tuning in order to obtain a satisfactory convergence rate. 


== 2.5 Optimistic Initial values 
All the methods we have discussed so far are dependent to some extent on
the initial action-value estimates, $Q_1 (a)$.
In the language of statistics, these methods are biased by their initial estimates.
For the sample-average methods,the bias disappears once all actions have been selected at least once, but for
methods with constant α, the bias is permanent, though decreasing over time. 
The bias are ussually not a problem and can be very useful but the downsize
is that the initial estimates ,becomes, in effect a set of parameters that must \
be picked by the user.

Initial action values can also be used as a simple way of encouraging ex-
ploration.We call this technique, *Optimistic Initial Values*.
We regard it as a simple trick that can be
quite eﬀective on stationary problems, but it is far from being a generally use-
ful approach to encouraging exploration. For example, it is not well suited to
nonstationary problems because its drive for exploration is inherently tempo-
rary. If the task changes, creating a renewed need for exploration, this method
cannot help. Indeed, any method that focuses on the initial state in any special
way is unlikely to help with the general nonstationary case. The beginning
of time occurs only once, and thus we should not focus on it too much. This
criticism applies as well to the sample-average methods, which also treat the
beginning of time as a special event, averaging all subsequent rewards with
equal weight.

== 2.6 Upper-Confidence-Bound Action selection 



Exploration is needed because the estimates of the action values are uncertain.
The greedy actions are those that look best at present, but some of the other
actions may actually be better. $epsilon$-greedy action selection forces the non-greedy
actions to be tried, but indiscriminately, with no preference for those that are
nearly greedy or particularly uncertain. It would be better to select among
the non-greedy actions according to their potential for actually being optimal,
taking into account both how close their estimates are to being maximal and
the uncertainties in those estimates. One eﬀective way of doing this is to select
actions as
$
  A_t = "arg"max_a [Q_t (a) + c sqrt( (ln t) / (N_t (a)) )]
$

if $N_t (a) = 0$ then $a$ is considered to be a maximizing action. 

The idea of this *upper confidence bound* (UCB) action selection is 
that the quare-root term is a measure of the uncertainty or variance in the estimate
of a’s value. The quantity being max’ed over is thus a sort of upper bound
on the possible true value of action a, with the c parameter determining the
conﬁdence level. Each time a is selected the uncertainty is presumably reduced;
$N_t (a)$ increases and the uncertainty measure term is decreased. On the other
hand, each time that action other $a$ is selected. $t$ is increased and the uncertainty 
estimate increases too. The natural log makes the increase a bit smaller 
over time but is unbounded; all actions will eventually be selected , 
but as time goes by it will be a longer wait and thus a lower selection 
frequency,for actions with a lowert value-estimate or that have already been selected 
more times.

UCB
will often perform well, as shown here, but is more diﬃcult than $epsilon$-greedy
to extend beyond bandits to the more general reinforcement learning settings
general reinforcement learning settings
considered in the rest of this book. One diﬃculty is in dealing with nonsta-
tionary problems; something more complex than the methods presented in
Section 2.4 would be needed.
Another diﬃculty is dealing with large state
spaces, particularly function approximation.

== 2.7 Gradient Bandits 

Instead of considered methods that estimate action values
and use those estimates to select actions;in this section , we consider learning *numerical preference $H_t (a)$* for 
each action a.
The larger the preference, the more often the action is taken but 
the preference has no interpretation in terms of the rewards. 
Only the relative preference of one action over another is
important; if we add 1000 to all the preferences there is no aﬀect on the action
probabilities, which are determined according to a soft-max distribution (i.e.,
Gibbs or Boltzmann distribution) as follows:

$
Pr{A_t = a } = (e^(H_t (a))) / (sum_(b=1)^n e^(H_t(b))) = pi_t (a)
$

$pi_t (a)$ is a new notation for the probability of taking action $a$ 
at time $t$. Initially are preferences are the same , $H_1 (a) = 1 "  " forall a$
so that all actions have equal probability of being selected. 

A natural learning algorithm for this setting based on the idea 
of stochastic gradient ascent; on each step, after selecting the action 
$A_t$ and receiving the reward $R_t$, theb preferences are updated by: 
$
  H_(t + 1) &= H_t(A_t) + alpha (R_t - macron(R)_t (1 - pi_t (A_t)).  "   and"\
  H_(t + 1) &= H_t(a) - alpha (R_t = macron(R)_t) pi_t (A) 

$

where $alpha  > 0$ is a step size param , and $macron(R)_t in RR$ is the 
average of all the rewards up through and including $t$, which can 
be computed incrementally using the methods in section 2.3 or 2.4 if the problem 
is nonstationary. The $macron(R)_t$ term serves as a baseline with which the reward is compared. If the
reward is higher than the baseline, then the probability of taking
$A_t$ At in the future is increased, and if the reward is below baseline, then probability is
decreased. The non-selected actions move in the opposite direction.

One can gain a deeper insight into this algorithm by understanding it as
a stochastic approximation to gradient ascent. In exact *gradient ascent*, 
each preference $H_t (a)$ would be incrementing proportional to the increment’s eﬀect
on performance:
$
H_(t + 1) (a) = H_t(a)  + alpha  (partial EE [R_t])/ (partial H_t (a)).
$

where the measure of the increment here is the expected reward. 
$
EE [R_t] = sum_b pi_t (b) q(b)
$

It is not posssible to implement gradient ascent exactly in our case 
we do not know the $q(b)$ but infact the updates of our algorithm 0.11 
are equal to 0.12 in expected value, making the algorithmn an instance 
of *stochastic gradient ascent*:\

#proof[
  #unnumbered(
$
  (partial EE [R_t])/ (partial H_t (a))
        &= partial / (partial H_t (a)) [ sum_b pi_t (b)  q(b)]\
        &= sum_b  q(b) (partial pi_t (b))/ (partial H_t (a))\
        &= sum_b  (q(b) - X_t) (partial pi_t (b))/ (partial H_t (a))\
$
)
  where $X_t$ can be any scalar that does not depend on $b$, we can 
  include it here because the gradiet sum to zero over all the actions, 
  $sum_b (partial pi_t (b)) / (partial H_t (a)) = 0$. As $H_t$ is changed , some 
  actions'  probabilities  go up and some go down but the sum of 
  changes must be zero because the sum of the probabilities remain 1.
  
  #unnumbered(
    $
     &= sum_b pi_t (b) (q(b) - X_t) (partial pi_t(b))/ (partial H_t (a)) "/" pi_t (b) \
    $

  )

The equation is now in the form of an expectation, summing over all possible
values $b$ of the random variable $A_t$ , then multiplying by the probability of
taking those values. Thus:
 
  #unnumbered(  
$
   &= EE[ (q (A_t) - X_t) (partial pi_t (A_t))/ (partial H_t (a)) "/" pi_t (A_t)\]\
   &= EE[ (R_t - macron(R)_t) (partial pi_t (A_t))/ (partial H_t (a)) "/" pi_t (A_t)\]

$
  )

  we chose $X_t = R_T$ and substitute $R_t$ for $q(A_t)$, which is 
  permitted because $EE (R_t) = q(A_t)$ because all the other factors 
  are non random.

  We later show that $ (partial pi_t (b)) / ( partial H_t (a)) = pi_t (b) ( II_(a=b)-pi_t (a)) $
  where $II_(a=b) = 1 "if" a=b, "else" 0$\ 
  Assuming that for now we have: 
  $
      &= EE [( R_t - macron(R)_t) pi_t (A_t) ( II_(a=b) - pi_t (a)) "/" pi_t (A_t)]\
      &= EE [( R_t - macron(R)_t) ( II_(a=b) - pi_t (a))]
  
  $

Our plan has been to write the performance gradient as an expecta-
tion of something that we can sample on each step, as we have just done, and
then update on each step proportional to the sample. Substituting a sample
of the expectation above for the performance gradient, yieds :\
  $
  H_(t+1) = H_t (a) + alpha (R_t - macron(R)_t)(II_(a=A_t) - pi_t (a)), "  " forall a
  $

  Which is equivalent to the original algorithm. 

We now show that: 
$ (partial pi_t (b)) / ( partial H_t (a)) = pi_t (b) ( II_(a=b)-pi_t (a)) $


]

== Associative Search 
So far in this chapter we have considered only nonassociative tasks, in which
there is no need to associate diﬀerent actions with diﬀerent situations. In
these tasks the learner either tries to ﬁnd a single best action when the task is
stationary, or tries to track the best action as it changes over time when the
task is nonstationary. However in general RL task ,there is more 
than one situation and the goal is to learn a *policy* : a mapping 
from situations to the actions that are best in those situations.

Here is a simple way in which nonassociative tasks extend to associate setting. 
suppose there are several diﬀerent n-armed bandit tasks,
and that on each play you confront one of these chosen at random. Thus, the
bandit task changes randomly from play to play. This would appear to you as
a single, nonstationary n-armed bandit task whose true action values change
randomly from play to play. You could try using one of the methods described
in this chapter that can handle nonstationarity, but unless the true action
values change slowly, these methods will not work very well. Now suppose,
however, that when a bandit task is selected for you, you are given some
distinctive clue about its identity (but not its action values). Maybe you are
facing an actual slot machine that changes the color of its display as it changes
its action values. Now you can learn a policy associating each task, signaled
by the color you see, with the best action to take when facing that task—for
instance, if red, play arm 1; if green, play arm 2. With the right policy you
can usually do much better than you could in the absence of any information
distinguishing one bandit task from another.



















