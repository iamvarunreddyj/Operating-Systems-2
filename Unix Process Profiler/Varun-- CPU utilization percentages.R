## Graph - Memory ##

time <- c(0, 1, 2, 3, 4, 5, 6, 7, 8)
process_utilization <- c(0.060536, 0.329672, 0.049317, 0.049851, 0.041057, 0.036688, 0.050382, 0.836208, 0.034256)

plot(time, process_utilization, type='o', pch=2, col="blue", xlab="Interval (2 Seconds)", ylab="Process Utilization (%ge of each interval)", ylim=c(0, 1.0), xlim=c(0,9)) 
# lines(time, heap_length, type='o', pch=20, col="red", lty=2) 
legend("topleft", c("Utilization Time (%ge of each interval"), cex=0.8, col=c("blue","red"), lty=1:2, lwd=2, bty="n")