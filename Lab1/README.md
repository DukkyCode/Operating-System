# Measurement Assignment Questions
You may add your answers to the assignment questions in the space provided
below.  This is a markdown file (.md), so you may use markdown formatting. If
you do not know about markdown (or know but don't care to format anything) then
you may just treat this as a plain text file.

# Collaboration Policy
On this assignment, you are allowed to collaborate with one or more classmates
on answering the questions in **Part A** _only_.  You are also allowed to share
the times you get for each measurement with your classmates to compare and see
if you're getting similar results.

However, you must write your own code and debug it _individually_.  You may not
share your code with anyone else, nor look at anyone else's code (including the
usual class academic integrity policy regarding copying code from online).  You
must also answers the questions in **Part B** _individually_.  No collaboration
is allowed on **Part B** _except_ for sharing the measurement times you got.


# Part A
Answer these questions before you start coding.  You may discuss the **Part A
questions _only_** with your classmates, but you must write your own answers
in your own words.


## Question 1
Research the `gettimeofday()` function.  (`man gettimeofday`)  What does this
function do?  How would you use it to measure the amount of time required
("latency" or "delay") to perform an action (say, calling the `foo()` function)?

### Answer:


## Question 2
Now research the `getrusage()` function (`man getrusage`), paying particular
attention to the `ru_utime` and `ru_stime` fields in the result.  (Assume it's
being called with the `RUSAGE_SELF` argument.)  How are the user time
(`ru_utime`), system time (`ru_stime`), and the time returned by
`gettimeofday()` all different from each other?

### Answer:


## Question 3
Suppose you want to measure the time it takes to do one fast thing; something
that takes less than a minute to do (e.g., fill one glass with water).  However,
the only tool you have for taking measurements is a digital clock (regular
clock, not a stopwatch) that does not show seconds, only shows hours and
minutes.  How would you get a reliable measure of how long it takes to do the
thing once?  (Hint: Thinking about whether this is a matter of accuracy or
precision will help you to answer the next question.)

### Answer:


## Question 4
Suppose you want to find the average amount of time required to run function
`foo()`.  What is the difference between the following two approaches?  Which
one is better, and why?  (You may assume `foo()` is _very_ fast.)
(Hint: Think about whether `foo()` being fast causes a problem with accuracy
or precision.)

```c
latency = 0
loop N times
    measure start time
    call foo()
    measure end time
    latency += end time - start time
print latency/N
```

```c
measure start time
loop N times
    call foo()
measure end time
print (end time - start time) / N
```

### Answer:


## Question 5
Consider the following code.  What work is this code doing, besides calling
`foo()`?

```c
int i;
for (i = 0; i < N; ++i) {
  foo();
}
```

### Answer:


## Question 6
If `foo()` is very fast, and the time to perform `for (i = 0; i < N; ++i)` may
be significant (relative to the time needed to call `foo()`), how could you make
your final measurement value includes *only* the average time required to call
`foo()`?  (Hint: Think about whether the time to perform
`for (i = 0; i < N; ++i)` causes a problem with accuracy or precision.)

### Answer:


# Part B
Before proceeding, write your code, debug it, and run your experiments.

Now that you've run all your experiments, answer the questions in "Part B".
You should **complete these questions on your own**, without discussing the
answers with anyone (unless you have questions for the course staff, of course).
Each question should only require approximately _a couple sentences_ to answer
properly, so _don't_ write an entire essay that isn't needed.


## Question 7
What was your general strategy for getting good measurements for all the
items?  (i.e., things you did in common for all of them, not the one-off
adjustments you had to figure out just for specific ones)

### Answer:


## Question 8
What measurement result did you get for each of the six measurements?  Based on
these results, which functions do you think are system calls (syscalls) and
which do you think are not?  (Provide your times either in seconds in
exponential notation, or in microseconds as an integer value.  Write the units.)

### Answer:

* allocate one page of memory with `mmap()`
  * user time:       **[ANSWER HERE]**
  * system time:     **[ANSWER HERE]**

* lock a mutex with `pthread_mutex_lock()`
  * user time:       **[ANSWER HERE]**
  * system time:     **[ANSWER HERE]**

* writing 4096 Bytes directly (bypassing the disk page cache) to /tmp
  * wall-clock time: **[ANSWER HERE]**

* reading 4096 Bytes directly (bypassing the disk page cache) from /tmp
  * wall-clock time: **[ANSWER HERE]**

* writing 4096 Bytes to the disk page cache
  * wall-clock time: **[ANSWER HERE]**

* reading 4096 Bytes from the disk page cache
  * wall-clock time: **[ANSWER HERE]**

* Syscalls:     **[LIST FUNCTIONS HERE]**
* Not syscalls: **[LIST FUNCTIONS HERE]**


## Question 9
What is the memory page size?  (i.e., that you used with `mmap`)

### Answer:


## Question 10
How did you deal with the problem of not being able to lock a mutex
more than once without unlocking it first?

### Answer:


## Question 11
Was performance affected by whether a file access operation is a read or write?
If so, which is faster?

### Answer:


## Question 12
What affect did the disk page cache have on file access performance?  Did it
affect reads and write differently?

### Answer:
