#import "jotter-notes.typ": *

= chapter 1 - RL Problem

#let hs(s) = text(fill: rgb("#ff0600"), s)

== RL
- RL problems involve learning what to do, how to map
  situations to actions to maximize a numerical reward signal
- The learner is not told what actions to take 
  but must figure out which actions yield the most 
  results by testing them out. 
- These actions may not affect the immediate reward 
  but also the next situations and , through that, all subsequent
  results.
- The basic idea for a full specification of RL problems 
  is simply to capture the most important aspects of 
  the real problem  facing a learning agent 
  interacting with its environment to achieve a goal
- The agent must be able to 1) sense the environment to some extent 
   2) able to take some actions that affect the said environment. 
   3) have a goal or goals relating to the state of the environment.
- The formulation is intended to include these 3 aspects , sensation , action 
  and goal, in their simplest possible form without trivializing 
  any of them.
- RL is different from #hs[supervised learning], which is ,
  learning from a training set of labeled examples provided by 
  a knowledgable external supervisor. Each example is 
  a description of the situation together with a a specifical, the 
  label, of the correct action the system should take in that situation.
  The big is for system to extrapolate or generalize its responses
  so that it acts correctly in situations that are not present in the 
  training set. 
- It is often impractical to obtain example of desired 
  behaviour that are both correct and representative of the 
  of all the situations in which the agent has to act. 
- RL is also different from what is  called #hs[unsupervised learning], 
  which is typically about finding structure in collections of unlabeled data. 
- Supervised learning and unsupervised learning appear to classify ML 
  paradigigms, but they do not. Although one may to try to think 
  of RL  asd a type of unsupervised learning as it also does not rely on a 
  examples of correct behaviour, but RL is trying to maximise the 
  reward signal instead of trying to find a hidden structure. 
- Uncovering the structure in an agent's experience can certainly 
  be useful in RL but it does not address the RL agent's problem of 
  maximising a reward signal. 
- One of challenges in RL is that of Exploration vs Exploitation. To obtain 
  a lot of reward the agent must  prefer actions it has taken in the past and found to have
  been effective in producing rewards. But to discover such actions, it has to 
  take actions that it has not taken before. 
- The agent has to *exploit* what it already knows in order to 
  obtain rewards and it has to *explore* in order to make better actions in the future.
  Either exploration nor Exploitation can be taken exclusievley without 
  failing at the task.  The agent must try a variety of actions
  and progressively favor that appear to be the best.
- Another key feature in RL is that it explicitly consider the WHOLE problem 
  of a goal-directed agent interacting with an uncertain environment. This is in 
  contrast with with many approaches that consider subproblmes without 
  addressing how they fit into a larger picture. 
- RL take the opposite stack,starting with a complete, interactive 
  goal seeking agent. ALL RL agents have explict  goal, sensation of the 
  environment and action to influence the environment. It is ussualy assumned 
  that the agent has to operate despite the significan certanity about the environment 
  it faces. Explicit goal in a sense that the agent can judge progress towards its goal 
  based on what it can sense directly.
- For planning, the agent has to address the interplay between the 
  planning and the real-time action section, as well as question how the environment models 
  are acquired and improved. When RL involves supervised learning it does 
  so for a specific reasons that determines which capabilities are critical and which 
  are not. Important subproblems should be isolated and studied but they 
  should be subproblems that play a clear role in the complete,interactive, 
  goal seeking agents even if all the details of the  complete agent 
  cannot yet be filled.

== Elements of RL 
Beyond the agent and the environment we can identify 4 main subelements 
of a RL system. a policy, a reward signal , a value function and optionally, a 
model of the environment.

#definition[
  A *policy* defines the learning agent's way of behaving at a given time.
  Roughly speaking, a *policy* is a mapping of perceived state of the 
  environment to actions to be taken in those states. 
]
In pyschology this would be called a set of stimulus-response rule or 
associations.
In some cases a policy may be a simple function or lookup table 

The policy is the core of a reinforcement learning agent in the sense that it alone
is suﬃcient to determine behavior. In general, policies may be stochastic.

#definition[
  A *reward signal* defines the goal in a RL problem. On each step 
  , the environment sends to the RL agent a single number, a *reward*. 
  The agent's sole objective is to maximize the total reward it receives 
  in the long run. The reward signal thus defines what are the good and bad events 
  for the agent.
]
The reward given to the agent at any given times depends on the agent's current 
action and the state of the agent's environment. The agent 
cannot alter the process that does this.The only way that the agent 
can influence the reward signal is through its actions , which can
have a direct impact on the reward or the an indirect effect 
through changing the environment's state.

The reward signal is the primary basis for altering the policy. If an action selected by the
policy is followed by low reward, then the policy may be changed to select
some other action in that situation in the future.

In general the reward signals may be stochastic functions of the state  of the 
environment and the actions taken.

#definition[
  A *value function* specifies what is good in the long run.
    Roughly speaking, the *value* of a state is the total amount of 
    rewards an agent can expect to accumulate over the future, starting from that state.

]

Wheareas the rewards determine the immediate, intrisic desirability of environment 
states, values indicate the long-term desirability of states after taking into account the 
the states that are likely to follow , and the rewards in that state. 
For example, a state might always yield a low immediate reward but still have a high value because 
it is regularly followed by other states that yield high rewards.


Rewards are in a sense primary, whereas values, as predictions of rewards,
are secondary. Without rewards there could be no values, and the only purpose
of estimating values is to achieve more reward.

It is values with which we are most concerned about when making 
and evaluating decisions.Action choices are made based on 
value judgements. We seeks actions that bring about highest values , not highest 
rewards because actions obtain the highest rewards for us over the long run.

In decision-making and planning, the derived quantity called value is the one with which we are most
concerned. Unfortunately, it is much harder to determine values than it is to
determine rewards.

Rewards are basically given directly by the environment,
but values must be estimated and re-estimated from the sequences of obser-
vations an agent makes over its entire lifetime

#definition[
  A *model* of the environment is something that mimics 
  the behavior of the environment, or more generally, that allow inferences 
  to be made  about how the environment will behave.
  For example, given a state and action,the model might predict the 
  resultant next state and next reward.
]
Models are used to planning, by which we mean any way of deciding 
on a course of action by considering possible future situations before they 
are actually experienced. 

RL methods that uses models and planning are called *model-based* methods.



== Limitations and Scope 
It is not strictly necessary to solve RL problems with 
methods centered around estimating the value functions. 

For example, methods such as
genetic algorithms, genetic programming, simulated annealing, and other opti-
mization methods have been used to approach reinforcement learning problems
without ever appealing to value functions. These methods evaluate the “life-
time” behavior of many non-learning agents, each using a diﬀerent policy for
interacting with its environment, and select those that are able to obtain the
most reward. We call these evolutionary methods because their operation is
analogous to the way biological evolution produces organisms with skilled be-
havior even when they do not learn during their individual lifetimes. If the
space of policies is suﬃciently small, or can be structured so that good policies
are common or easy to ﬁnd, or if a lot of time is available for the search, then
evolutionary methods can be eﬀective. In addition, evolutionary methods have
advantages on problems in which the learning agent cannot accurately sense
the state of its environment.


Evolutionary methods ignore much of
the useful structure of the reinforcement learning problem: they do not use
the fact that the policy they are searching for is a function from states to
actions; they do not notice which states an individual passes through during
its lifetime, or which actions it selects


When we say that a reinforcement learning agent’s goal is to maximize a nu-
merical reward signal, we of course are not insisting that the agent has to
actually achieve the goal of maximum reward. Trying to maximize a quantity
does not mean that that quantity is ever maximized. The point is that a re-
inforcement learning agent is always trying to increase the amount of reward
it receives. Many factors can prevent it from achieving the maximum, even if
one exists. In other words, optimization is not the same a optimality.

















