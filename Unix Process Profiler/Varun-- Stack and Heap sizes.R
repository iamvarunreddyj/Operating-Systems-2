## Graph - Memory ##

time <- c(0, 2, 4, 6, 8, 10, 12, 14, 16)
stack_length <- c(8192, 65536, 131072, 196608, 262144, 262144, 327680, 393216, 458752)
heap_length <- c(0, 127212, 192748, 258284, 323820, 389356, 520428, 913644, 913644)

plot(time, stack_length, type='o', pch=2, col="blue", xlab="TIME in Sec", ylab="Stack/Heap Sizes", ylim=c(0, 1200000), xlim=c(0,16)) 
lines(time, heap_length, type='o', pch=20, col="red", lty=2) 
legend("topleft", c("Stack","Heap"), cex=0.8, col=c("blue","red"), lty=1:2, lwd=2, bty="n")