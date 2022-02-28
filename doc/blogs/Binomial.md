# Drawing random values from from non-uniform distributions: A binomial case study

One common challenge in scientific computing is drawing from specific random distributions.
These can be time-consuming and hard to be acurate, especially when rare events are
important to include.

There are many different mathematical distributions to consider.  For any common distribution,
you should be able to find plenty of information about it on the internet.  There are a
handful of specific questions to ask:

1. Is there a simple, accurate conversion from a uniform [0.0, 1.0) distribution -- like those
produced by most random number generators -- to the distribution I need.

2. If not, how close of an approximation can I get?  Is it good enough?

3. If not, am I going to be using the same parameters over and over such that I can do some
pre-processing to produce a fast result?  (For example, am I using a fair 6-sided die and so I
know each outcome always has a 1/6 chance of showing up?)

4. If not, how much traditional optimization can I use in the brute-force calculation?

Here, I am going to focus on *Binomial Distributions* and some other related distributions,
but the logic that I use is applicable elsewhere.

As a reminder:

A **Binomial Distribution** asks: If an event is going to occur with probability *p* and we test
for it *N* times, how many times will the event actually occur?  *Example*: Each time an
programmer writes a line of code, there is a *p*=0.03 chance that she introduces a bug.  How many
bugs does she create after *N*=100 lines of code?

A **Negative Binomial Distribution** turns this around:  If an event is going to occur with
probability *p*, how many times do we need to test for it for it to actually occur *N* times.
*Example*: Given a *p*=0.03 chance of introducing a bug with each line of code, how many lines
would a programmer need to write to reach *N*=10 bugs?

A **Geometric Distribution** is a special case of the Negative Binomial Distribution where *N*=1.
*Example*: Given a *p*=0.03 chance of introducing a bug, how many lines can a programmer write
before introducing the next bug?

A **Poisson Distribution** is a continuous version of a Binomial Distribution, used for measuring
the number of independent events that occur in a time period rather than during a specified
number of events.