# D2NG-POMCP 1.0 

This is the code release of D2NG-POMCP as in the ICAPS 2014 paper
"Thompson Sampling based Monte-Carlo Planning in POMDPs",
by Aijun Bai, Feng Wu, Zongzhang Zhang, and Xiaoping Chen.

# Notes
In `run.sh`:

- Set `TIMEOUTPERACTION > 0` to run the code in anytime mode, where each action selection is running up to `TIMEOUTPERACTION` seconds
- Set `TIMEOUTPERACTION = -1` to disable anytime mode, where each action selection is running up to a fixed number of iterations

