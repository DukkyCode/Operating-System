# Measurement Assignment Questions

You may add your answers to the assignment questions in the space provided
below. This is a markdown file (.md), so you may use markdown formatting. If
you do not know about markdown (or know but don't care to format anything) then
you may just treat this as a plain text file.

# Collaboration Policy

On this assignment, you are allowed to collaborate with one or more classmates
on answering the questions in **Part A** _only_. You are also allowed to share
the times you get for each measurement with your classmates to compare and see
if you're getting similar results.

However, you must write your own code and debug it _individually_. You may not
share your code with anyone else, nor look at anyone else's code (including the
usual class academic integrity policy regarding copying code from online). You
must also answers the questions in **Part B** _individually_. No collaboration
is allowed on **Part B** _except_ for sharing the measurement times you got.

# Part A

Answer these questions before you start coding. You may discuss the **Part A
questions _only_** with your classmates, but you must write your own answers
in your own words.

## Question 1

Research the `gettimeofday()` function. (`man gettimeofday`) What does this
function do? How would you use it to measure the amount of time required
("latency" or "delay") to perform an action (say, calling the `foo()` function)?

### Answer:

The function gettimeofday(struct timeval *restrict tv, struct timezone *restrict tz) set the
time in timeval structure "tv" member. It will give time elapsed since the Epoch, which is 1970-01-01 00:00:00 +0000 (UTC). We will call the function gettimeofday() to get a start time before calling function 'foo()' and an end time after foo() is finished. The latency or delay will be measured by the subtraction of the start time and end time.

## Question 2

Now research the `getrusage()` function (`man getrusage`), paying particular
attention to the `ru_utime` and `ru_stime` fields in the result. (Assume it's
being called with the `RUSAGE_SELF` argument.) How are the user time
(`ru_utime`), system time (`), and the time returned by
`gettimeofday()` all different from each other?

### Answer:

The time returned by `gettimeofday()` will give you time elasped since the Epoch. Assuming you are getting the latency from `gettimeofday()` using the formula `(start_time - end_time)`, then the time from the latency will be the elapsed time of the process spend in both user mode and kernel mode. Moreover, if you have multiple processes running concurrently, it will include the time slices used by other processes and the time the process got interrupted(I/O). `ru_utime`, on the other hand, is only the amount of CPU time spent in user-mode within the process, and `ru_stime` is the CPU time spend in the kernel within the process.

## Question 3

Suppose you want to measure the time it takes to do one fast thing; something
that takes less than a minute to do (e.g., fill one glass with water). However,
the only tool you have for taking measurements is a digital clock (regular
clock, not a stopwatch) that does not show seconds, only shows hours and
minutes. How would you get a reliable measure of how long it takes to do the
thing once? (Hint: Thinking about whether this is a matter of accuracy or
precision will help you to answer the next question.)

### Answer:

In terms of measuring a job that takes less than a minute to do, with only a digital clock that can only show hours and minutes, it is difficult to measure it in terms of accuracy because you can never get the most accurate time of the job after the job is done. That's why for this matter, we can only focus on precision. We will start the clock and do the job for N times. Then, we stop the clock and measure the overall time to finish N job. Finally, we average the amount of time to get the time for each job.

## Question 4

Suppose you want to find the average amount of time required to run function
`foo()`. What is the difference between the following two approaches? Which
one is better, and why? (You may assume `foo()` is _very_ fast.)
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

The first approach is incrementing the elapsed time of foo() between each time it loops and compute the average. The second approach measures the total time taken by N calls to foo() and then divides by N to get the average time per call. The second approach should be better because it measures the total time taken of the entire loop as a single unit, which reduces the impact of any measurement overhead.

## Question 5

Consider the following code. What work is this code doing, besides calling
`foo()`?

```c
int i;
for (i = 0; i < N; ++i) {
  foo();
}
```

### Answer:

The code also performs `for (i = 0; i < N; ++i)` which it will increment the value of i, compare with N , and branch back to the top of the loop.

## Question 6

If `foo()` is very fast, and the time to perform `for (i = 0; i < N; ++i)` may
be significant (relative to the time needed to call `foo()`), how could you make
your final measurement value includes _only_ the average time required to call
`foo()`? (Hint: Think about whether the time to perform
`for (i = 0; i < N; ++i)` causes a problem with accuracy or precision.)

### Answer:

`for (i = 0; i < N; ++i)` can cause problem with both accuracy because it might take more time for the code to process the loop than the actual `foo()` function itself. You can include _only_ the average time required to call `foo()` by using approach 1 from question 4. Inside the loop, you will measure the elasped time of `foo()` and compute the the average time outside of the loop.

# Part B

Before proceeding, write your code, debug it, and run your experiments.

Now that you've run all your experiments, answer the questions in "Part B".
You should **complete these questions on your own**, without discussing the
answers with anyone (unless you have questions for the course staff, of course).
Each question should only require approximately _a couple sentences_ to answer
properly, so _don't_ write an entire essay that isn't needed.

## Question 7

What was your general strategy for getting good measurements for all the
items? (i.e., things you did in common for all of them, not the one-off
adjustments you had to figure out just for specific ones)

### Answer:

- To measure the start time right before the operation execution and end time right after the operation execution. Try to make sure that the elapsed time does not get affected by external factor.

## Question 8

What measurement result did you get for each of the six measurements? Based on
these results, which functions do you think are system calls (syscalls) and
which do you think are not? (Provide your times either in seconds in
exponential notation, or in microseconds as an integer value. Write the units.)

### Answer:

- allocate one page of memory with `mmap()`

  - user time: **0.000000 s**
  - system time: **0.000003 s**

- lock a mutex with `pthread_mutex_lock()`

  - user time: **0.000000 s**
  - system time: **0.000001 s**

- writing 4096 Bytes directly (bypassing the disk page cache) to /tmp

  - wall-clock time: **0.001115 s**

- reading 4096 Bytes directly (bypassing the disk page cache) from /tmp

  - wall-clock time: **0.000003 s**

- writing 4096 Bytes to the disk page cache

  - wall-clock time: **0.000017 s**

- reading 4096 Bytes from the disk page cache

  - wall-clock time: **0.000004 s**

- Syscalls: **[mmap(), open(), read(), write()]**
- Not syscalls: **[pthread_mutex_lock, posix_memalign(), malloc()]**

## Question 9

What is the memory page size? (i.e., that you used with `mmap`)

### Answer:

4096

## Question 10

How did you deal with the problem of not being able to lock a mutex
more than once without unlocking it first?

### Answer:

Everytime I initialize a mutex and lock, I will always unlock and destroy the mutex afterwards. In this assignment, however, there is only one mutex so the probem above does not occur if you unlock and destroy properly.

## Question 11

Was performance affected by whether a file access operation is a read or write?
If so, which is faster?

### Answer:

The performance of a file acesss operations can definitely be affected by whether the operation is a read or a write. In the above measurement, writing operation tends to be slower than the reading operation which can also be true in normal application.

## Question 12

What affect did the disk page cache have on file access performance? Did it
affect reads and write differently?

### Answer:

It can affect both read and write greatly because cache can be used to reduce the amount of time required to access the data, especially on read as you need subsequent reads of the same data. For the above experiment, write takes more time because it has to access main memory to fetch the data and write it to the file, which can be seen the time difference between the two experiments.
