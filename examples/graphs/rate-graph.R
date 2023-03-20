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
data <- read.table("rate-trace-cs4-v3.txt", header=T)
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

### every plot by node
#node zero
g.zero <- ggplot(data.zero, aes (x=FaceId, y=PacketRaw.sum, fill=Type)) +
  	geom_bar(stat="identity", position=position_dodge(), colour="black", size=.1) +
	scale_y_continuous('Nº of packets', limits=c(0, 13)) +
	scale_x_discrete("geoFace ID") +
  	facet_wrap(~ "Node 0") +
	theme(axis.text.x  = element_text(angle=0, hjust=0.5, size=11,colour="black")) + 
	theme(legend.position = "bottom")+ 
	labs(fill = "")

print(g.zero)
png("n0_geo_in_out.png", width=340, height=320)
print(g.zero)
retval <- dev.off()

#node one
g.one <- ggplot(data.one, aes (x=FaceId, y=PacketRaw.sum, fill=Type)) +
  	geom_bar(stat="identity", position=position_dodge(), colour="black", size=.1) +
	scale_y_continuous('Nº of packets', limits=c(0, 13)) +
	scale_x_discrete("geoFace ID") +
  	facet_wrap(~ "Node 1") +
	theme(axis.text.x  = element_text(angle=0, hjust=0.5, size=11,colour="black")) + 
	theme(legend.position = "bottom")+ 
	labs(fill = "")

print(g.one)
png("n1_geo_in_out.png", width=340, height=320)
print(g.one)
retval <- dev.off()

#node two
g.two <- ggplot(data.two, aes (x=FaceId, y=PacketRaw.sum, fill=Type)) +
  	geom_bar(stat="identity", position=position_dodge(), colour="black", size=.1) +
	scale_y_continuous('Nº of packets', limits=c(0, 13)) +
	scale_x_discrete("geoFace ID") +
  	facet_wrap(~ "Node 2") +
	theme(axis.text.x  = element_text(angle=0, hjust=0.5, size=11,colour="black")) + 
	theme(legend.position = "bottom") + 
	labs(fill = "")

print(g.two)
png("n2_geo_in_out.png", width=340, height=320)
print(g.two)
retval <- dev.off()

#node three
g.three <- ggplot(data.three, aes (x=FaceId, y=PacketRaw.sum, fill=Type)) +
  	geom_bar(stat="identity", position=position_dodge(), colour="black", size=.1) +
	scale_y_continuous('Nº of packets', limits=c(0, 13)) +
	scale_x_discrete("geoFace ID") +
  	facet_wrap(~ "Node 3") +
	theme(axis.text.x  = element_text(angle=0, hjust=0.5, size=11,colour="black")) + 
	theme(legend.position = "bottom")+ 
	labs(fill = "")

print(g.three)
png("n3_geo_in_out.png", width=340, height=320)
print(g.three)
retval <- dev.off()

#node four
g.four <- ggplot(data.four, aes (x=FaceId, y=PacketRaw.sum, fill=Type)) +
  	geom_bar(stat="identity", position=position_dodge(), colour="black", size=.1) +
	scale_y_continuous('Nº of packets', limits=c(0, 13)) +
	scale_x_discrete("geoFace ID") +
  	facet_wrap(~ "Node 4") +
	theme(axis.text.x  = element_text(angle=0, hjust=0.5, size=11,colour="black")) + 
	theme(legend.position = "bottom")+ 
	labs(fill = "")

print(g.four)
png("n4_geo_in_out.png", width=340, height=320)
print(g.four)
retval <- dev.off()

#node five
g.five <- ggplot(data.five, aes (x=FaceId, y=PacketRaw.sum, fill=Type)) +
  	geom_bar(stat="identity", position=position_dodge(), colour="black", size=.1) +
	scale_y_continuous('Nº of packets', limits=c(0, 13)) +
	scale_x_discrete("geoFace ID") +
  	facet_wrap(~ "Node 5") +
	theme(axis.text.x  = element_text(angle=0, hjust=0.5, size=11,colour="black")) + 
	theme(legend.position = "bottom") + 
	labs(fill = "")

print(g.five)
png("n5_geo_in_out.png", width=340, height=320)
print(g.five)
retval <- dev.off()

g.all <- ggplot(data.combined, aes (x=FaceId, y=PacketRaw.sum, fill=Type)) +
  	geom_bar(stat="identity", position=position_dodge(), colour="black", size=.1) +
	scale_y_continuous('Nº of packets', limits=c(0, 13)) +
	scale_x_discrete("geoFace ID") +
  	facet_wrap(~ "Node"+Node) +
	theme(axis.text.x  = element_text(angle=0, hjust=0.5, size=11,colour="black"))

print(g.all)
png("geo_in_out.png", width=480, height=320)
print(g.all)
retval <- dev.off()

#g.all <- ggplot(data.combined) +
#  geom_point(aes (x=Type, y=PacketRaw.sum, color=Type), size=1) +
#  scale_y_continuous('Nº of packets', limits=c(0, 15)) +
#  facet_wrap("node" ~Node) +
#  theme(axis.text.x = element_text(angle=-90, vjust=0.6 ,hjust=0.5, size=9,colour="black"))

#print(g.all)

#aes (x=Type, y=PacketRaw.sum, color=Type), size=1

delay_dat <- read.table("app-delays-trace-cs4-v3.txt", header=T)
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
