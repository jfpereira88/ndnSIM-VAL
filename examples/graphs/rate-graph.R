# Copyright (c) 2012,2015  Alexander Afanasyev <alexander.afanasyev@ucla.edu>


#install.packages('ggplot2')
library(ggplot2)
#install.packages('scales')
library(scales)

#install.packages('doBy')
library(doBy)

#########################
# Rate trace processing #
#########################
data <- read.table("rate-trace-cs4-v2.txt", header=T)
data$Node <- factor(data$Node)
data$FaceDescr <- factor(data$FaceDescr)
data$FaceId <- factor(data$FaceId)
data$Type <- factor(data$Type)


# exlude irrelevant types
data = subset(data, FaceDescr %in% c("geoFace://"))
data = subset(data, Type %in% c("OutInterests", "InInterests", "InData", "OutData"))


# combine stats from all faces
data.combined = summaryBy(. ~ Node + FaceId + Type, data=data, FUN=sum)

data.zero = subset (data.combined, Node == "0")
data.one = subset (data.combined, Node == "1")
data.two = subset (data.combined, Node == "2")
data.three = subset (data.combined, Node == "3")
data.four = subset (data.combined, Node == "4")
data.five = subset (data.combined, Node == "5")
data.zero$PacketRaw[3] = 0
data.zero$PacketRaw.sum[3] = 0

print(data.combined)

# graph rates on all nodes in Kilobits
#g <- ggplot(data.combined, aes(x = Node, y = PacketRaw.sum, fill=Type)) +
#	geom_col(position='dodge', colour='black') +
#	theme_bw() + 
#	scale_y_continuous('Nº of packets', limits=c(0, 20))
    	

#print(g)

g.all <- ggplot(data.combined, aes (x=FaceId, y=PacketRaw.sum, fill=Type)) +
  	geom_bar(stat="identity", position=position_dodge(), colour="black", size=.1) +
	scale_y_continuous('Nº of packets', limits=c(0, 23)) +
	scale_x_discrete("geoFace ID") +
  	facet_wrap(~ "Node"+Node) +
	theme(axis.text.x  = element_text(angle=0, hjust=0.5, size=11,colour="black"))

print(g.all)
png("geo_in_out.png", width=480, height=280)
print(g.all)
retval <- dev.off()

#g.all <- ggplot(data.combined) +
#  geom_point(aes (x=Type, y=PacketRaw.sum, color=Type), size=1) +
#  scale_y_continuous('Nº of packets', limits=c(0, 15)) +
#  facet_wrap("node" ~Node) +
#  theme(axis.text.x = element_text(angle=-90, vjust=0.6 ,hjust=0.5, size=9,colour="black"))

#print(g.all)

#aes (x=Type, y=PacketRaw.sum, color=Type), size=1

delay_dat <- read.table("app-delays-trace-cs4-v2.txt", header=T)
delay_dat$Node <- factor(delay_dat$Node)
delay_dat$SeqNo <- factor(delay_dat$SeqNo)
delay_dat$Type <- factor(delay_dat$Type)
delay_dat$HopCount <- factor(delay_dat$HopCount)
delay_dat$RetxCount <- factor(delay_dat$RetxCount)

delay_dat = subset(delay_dat, Type %in% c("FullDelay"))
y_axe<-c(0:0.3)

g.delay <- ggplot(delay_dat) +
  	geom_point(aes (x=SeqNo, y=DelayUS/1000, color=Type), size=2) + 
 	scale_y_continuous('Delay (ms)', limits=c(45, 55)) +
	scale_x_discrete("Interest Seq Nº") + 
	ggtitle("Consumer App delays") +
	theme(plot.title = element_text(hjust=0.5), legend.position = "none")
	

print(g.delay)
png("app_delay.png", width=350, height=250)
print(g.delay)
retval <- dev.off()
