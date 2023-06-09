# Agents

**By TopchetoEU**

This is a program, written in C, that will find a reasonably fast path for N
agents to their targets in a field with obstacles. This is achieved using a
breadth first traversal algorithm, with the combination of a simple algorithm
that will move agents out of the way of others, when they're blocking the way.
Agents will prioritize making way for other agents, instead of going for their
target. This will allow, in general, a faster solve, since the agents are
working together.

## How to compile

Run the following command:

```sh
# Linux
gcc main.c -o agent
# Windows
gcc main.c -o agent.exe -DWIN32
```

This will output an executable in your environment, which you can run

## Command-line arguments

The program accepts two arguments, the latter of which is optional:

- Map filename - the location of the file, from which the map is going to be read
- Interval - defaults to 1000, the amount of milliseconds between steps

## Map syntax

Any file can be read as a map, however, only the following characters are meaningful:
- \# - A wall, trough which no agent can pass
- 1-9 - An agent. The program will break if agents are specified sparsely
- A-I - Targets, alphabetically corresponding to agents
- Anything else - Considered a free space

Note that the maximum size of a map is 32x32, and anything else is going to be
truncated. This is so to prevent dynamic memory allocation, as well as keeping
the underlying algorithm running smoothly.
