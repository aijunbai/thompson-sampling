# D2NG-POMCP 1.0 

This is the release of D2NG-POMCP used in the ICAPS 2014 paper
"Thompson Sampling based Monte-Carlo Planning in POMDPs",
by Aijun Bai, Feng Wu, Zongzhang Zhang, and Xiaoping Chen.

This release is developed based on ``POMCP'' - http://www0.cs.ucl.ac.uk/staff/d.silver/web/Applications_files/pomcp-1.0.tar.gz

# Notes
In `run.sh`:

- Set `TIMEOUTPERACTION > 0` to run the code in anytime mode, where each action selection is running up to `TIMEOUTPERACTION` seconds
- Set `TIMEOUTPERACTION = -1` to disable anytime mode, where each action selection is running up to a fixed number of iterations

