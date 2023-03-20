library(ggplot2)
library(scales)
library(plyr)

args = commandArgs(trailingOnly=TRUE)

if(length(args) != 2) {
  stop("needs sim csv file and confidence level.", call.=FALSE)
}

simDt <- read.csv(args[1], header=TRUE)
confidence = as.numeric(args[2])

filename <- paste("Plots-", confidence, ".pdf", sep = "")
pdf(file=filename)


## Satisfied Interest Ratio (SIR)
## VAL OUT INT percentage (VOIp)
## VAL OUT DATA percentage (VODp)
## NFD OUT INT percentage (NOIp)
## NFD OUT DATA percentage (NODp)
## GEOFACEp (GEOp) - percentage of out int with destination
## DATA ARRIVED (DAp)
sirSUMM <- ddply(simDt, c("N_VEH"), summarise,
            N    = length(SATISFIED_INT_RACIO),
            TYPE = c("SIR"),
            MEAN = round(mean(SATISFIED_INT_RACIO), 3),
            SD = round(sd(SATISFIED_INT_RACIO), 3),
            MARGIN  = round(qt(confidence, df= N-1) * (SD/sqrt(N)), 3),
            LB = round(MEAN - MARGIN, 3),
            HB = round(MEAN + MARGIN, 3)
)

voipSUMM <- ddply(simDt, c("N_VEH"), summarise,
            N    = length(Val_Out_Int_p),
            TYPE = c("VOIp"),
            MEAN = round(mean(Val_Out_Int_p), 3),
            SD   = round(sd(Val_Out_Int_p), 3),
            MARGIN = round(qt(confidence, df= N-1) * (SD/sqrt(N)), 3),
            LB = round(MEAN - MARGIN, 3),
            HB = round(MEAN + MARGIN, 3)
)
vodpSUMM <- ddply(simDt, c("N_VEH"), summarise,
            N    = length(Val_Out_Data_p),
            TYPE = c("VODp"),
            MEAN = round(mean(Val_Out_Data_p), 3),
            SD   = round(sd(Val_Out_Data_p), 3),
            MARGIN = round(qt(confidence, df= N-1) * (SD/sqrt(N)), 3),
            LB = round(MEAN - MARGIN, 3),
            HB = round(MEAN + MARGIN, 3)
)

noipSUMM <- ddply(simDt, c("N_VEH"), summarise,
            N    = length(Nfd_Out_Int_p),
            TYPE = c("NOIp"),
            MEAN = round(mean(Nfd_Out_Int_p), 3),
            SD   = round(sd(Nfd_Out_Int_p), 3),
            MARGIN = round(qt(confidence, df= N-1) * (SD/sqrt(N)), 3),
            LB = round(MEAN - MARGIN, 3),
            HB = round(MEAN + MARGIN, 3)
)

nodpSUMM <- ddply(simDt, c("N_VEH"), summarise,
            N    = length(Nfd_Out_Data_p),
            TYPE = c("NODp"),
            MEAN = round(mean(Nfd_Out_Data_p), 3),
            SD   = round(sd(Nfd_Out_Data_p), 3),
            MARGIN = round(qt(confidence, df= N-1) * (SD/sqrt(N)), 3),
            LB = round(MEAN - MARGIN, 3),
            HB = round(MEAN + MARGIN, 3)
)

geoSUMM <- ddply(simDt, c("N_VEH"), summarise,
            N    = length(Geoface_p),
            TYPE = c("GEOp"),
            MEAN = round(mean(Geoface_p), 3),
            SD   = round(sd(Geoface_p), 3),
            MARGIN = round(qt(confidence, df= N-1) * (SD/sqrt(N)), 3),
            LB = round(MEAN - MARGIN, 3),
            HB = round(MEAN + MARGIN, 3)
)

ddrSUMM <- ddply(simDt, c("N_VEH"), summarise,
            N    = length(Data_arrived_p),
            TYPE = c("DDR"),
            MEAN = round(mean(Data_arrived_p), 3),
            SD   = round(sd(Data_arrived_p), 3),
            MARGIN = round(qt(confidence, df= N-1) * (SD/sqrt(N)), 3),
            LB = round(MEAN - MARGIN, 3),
            HB = round(MEAN + MARGIN, 3)
)

intSupByNFD <- ddply(simDt, c("N_VEH"), summarise,
            N    = length(Data_arrived_p),
            TYPE = c("ISNFD"),
            MEAN = round(mean((NFD_IN_INT - NFD_OUT_INT) / VAL_IN_INT), 3),
            SD   = round(sd((NFD_IN_INT - NFD_OUT_INT) / VAL_IN_INT), 3),
            MARGIN = round(qt(confidence, df= N-1) * (SD/sqrt(N)), 3),
            LB = round(MEAN - MARGIN, 3),
            HB = round(MEAN + MARGIN, 3)
)

intSupByVAL <- ddply(simDt, c("N_VEH"), summarise,
            N    = length(Data_arrived_p),
            TYPE = c("ISVAL"),
            MEAN = round(mean(((NFD_OUT_INT - VAL_OUT_INT) + (VAL_IN_INT - NFD_IN_INT))/ VAL_IN_INT), 3),
            SD   = round(sd(((NFD_OUT_INT - VAL_OUT_INT) + (VAL_IN_INT - NFD_IN_INT))/ VAL_IN_INT), 3),
            MARGIN = round(qt(confidence, df= N-1) * (SD/sqrt(N)), 3),
            LB = round(MEAN - MARGIN, 3),
            HB = round(MEAN + MARGIN, 3)
)

dataSupByNFD <- ddply(simDt, c("N_VEH"), summarise,
            N    = length(Data_arrived_p),
            TYPE = c("DSNFD"),
            MEAN = round(mean((NFD_IN_DATA - NFD_OUT_DATA) / VAL_IN_DATA), 3),
            SD   = round(sd((NFD_IN_DATA - NFD_OUT_DATA) / VAL_IN_DATA), 3),
            MARGIN = round(qt(confidence, df= N-1) * (SD/sqrt(N)), 3),
            LB = round(MEAN - MARGIN, 3),
            HB = round(MEAN + MARGIN, 3)
)

dataSupByVAL <- ddply(simDt, c("N_VEH"), summarise,
            N    = length(Data_arrived_p),
            TYPE = c("DSVAL"),
            MEAN = round(mean(((NFD_OUT_DATA - VAL_OUT_DATA) + (VAL_IN_DATA - NFD_IN_DATA))/ VAL_IN_DATA), 3),
            SD   = round(sd(((NFD_OUT_DATA - VAL_OUT_DATA) + (VAL_IN_DATA - NFD_IN_DATA))/ VAL_IN_DATA), 3),
            MARGIN = round(qt(confidence, df= N-1) * (SD/sqrt(N)), 3),
            LB = round(MEAN - MARGIN, 3),
            HB = round(MEAN + MARGIN, 3)
)



intRatios <- rbind(sirSUMM, voipSUMM)
intRatios <- intRatios[order(intRatios$N_VEH),]
intRatios$N_VEH <- factor(intRatios$N_VEH)
#intRatios

intDroppedRatios <- rbind(intSupByNFD, intSupByVAL)
intDroppedRatios <- intDroppedRatios[order(intDroppedRatios$N_VEH),]
intDroppedRatios$N_VEH <- factor(intDroppedRatios$N_VEH)


#dataRatios
dataRatios <- rbind(ddrSUMM, vodpSUMM)
dataRatios <- dataRatios[order(dataRatios$N_VEH),]
dataRatios$N_VEH <- factor(dataRatios$N_VEH)


dataDroppedRatios <- rbind(dataSupByNFD, dataSupByVAL)
dataDroppedRatios <- dataDroppedRatios[order(dataDroppedRatios$N_VEH),]
dataDroppedRatios$N_VEH <- factor(dataDroppedRatios$N_VEH)



## NFD GEOFACES (geoSUMM)
### DISC - OUT INT without DESTINATION, in DISCOVERY
### DEST - OUT INT with DESTINATION
discGeo <- ddply(simDt, c("N_VEH"), summarise,
               N    = length(NFD_INT_DISCOV),
               TYPE = "DISC",
               MEAN = round(mean((NFD_INT_DISCOV)/NFD_OUT_INT),3),
               SD   = round(sd((NFD_INT_DISCOV)/NFD_OUT_INT),3),
               MARGIN = round(qt(confidence, df= N-1) * (SD/sqrt(N)),3),
               LB = MEAN - MARGIN,
               HB = MEAN + MARGIN
)

destGeo <- ddply(simDt, c("N_VEH"), summarise,
               N    = length(NFD_INT_DEST),
               TYPE = "DEST",
               MEAN = round(mean((NFD_INT_DEST)/NFD_OUT_INT),3),
               SD   = round(sd((NFD_INT_DEST)/NFD_OUT_INT),3),
               MARGIN = round(qt(confidence, df= N-1) * (SD/sqrt(N)),3),
               LB = MEAN - MARGIN,
               HB = MEAN + MARGIN
)

geoStory <- rbind(discGeo, destGeo)
geoStory <- geoStory[order(geoStory$N_VEH),]
geoStory$N_VEH <- factor(geoStory$N_VEH)


# DELAYS
fullDelay <- ddply(simDt, c("N_VEH"), summarise,
               N    = length(FULLDELAY_MEAN),
               TYPE = "FULLDELAY",
               MEAN = round(mean(FULLDELAY_MEAN), 3),
               SD   = round(sd(FULLDELAY_MEAN), 3),
               MARGIN = round(qt(confidence, df= N-1) * (SD/sqrt(N)), 3),
               LB = MEAN - MARGIN,
               HB = MEAN + MARGIN
)

lastDelay <- ddply(simDt, c("N_VEH"), summarise,
               N    = length(LASTDELAY_MEAN),
               TYPE = "LASTDELAY",
               MEAN = round(mean(LASTDELAY_MEAN), 3),
               SD   = round(sd(LASTDELAY_MEAN), 3),
               MARGIN = round(qt(confidence, df= N-1) * (SD/sqrt(N)), 3),
               LB = MEAN - MARGIN,
               HB = MEAN + MARGIN
)

delayStory <- rbind(fullDelay, lastDelay)
delayStory <- delayStory[order(delayStory$N_VEH),]
delayStory$N_VEH <- factor(delayStory$N_VEH)


##### PLOTS ######

# RATIOS #

intRatiosPlot <- ggplot(data=intRatios, aes(x=N_VEH, , y=MEAN, group=TYPE, colour=TYPE))+
    theme(axis.title = element_text(size = 14)) +
    theme(axis.text = element_text(size = 16)) +
    theme(plot.title = element_text(size = 16)) +
    theme(legend.text = element_text(size = 14)) +
    theme(legend.title = element_text(size = 14)) +
    geom_line(linetype=2, size=0.1) +
    geom_point(aes(shape=TYPE), size=4.5) +
    geom_text(aes(label=MEAN), vjust=-0.7, hjust=1.4, size=3)+
    geom_errorbar(aes(ymin=LB, ymax=HB), linetype=1, size=0.6, width=0.35) +
    scale_y_continuous('Ratio')+
    scale_x_discrete("Nº of Vehicles in SIMs") +
    ggtitle("Interest Ratios") +
    scale_colour_discrete(name  ="",
                            breaks=c("SIR","VOIp"),
                            labels=c("Satisfied\nInt Ratio\n","\nSystem\nOut Int\nRatio")) +
    scale_shape_discrete(name  ="",
                           breaks=c("SIR","VOIp"),
                            labels=c("Satisfied\nInt Ratio\n","\nSystem\nOut Int\nRatio"))

intRatiosPlot

intDroppedRatiosPlot <- ggplot(data=intDroppedRatios, aes(x=N_VEH, , y=MEAN, group=TYPE, colour=TYPE))+
    theme(axis.title = element_text(size = 14)) +
    theme(axis.text = element_text(size = 16)) +
    theme(plot.title = element_text(size = 16)) +
    theme(legend.text = element_text(size = 14)) +
    theme(legend.title = element_text(size = 14)) +
    geom_line(linetype=2, size=0.1) +
    geom_point(aes(shape=TYPE), size=4.5) +
    geom_text(aes(label=MEAN), vjust=-0.7, hjust=1.4, size=3)+
    geom_errorbar(aes(ymin=LB, ymax=HB), linetype=1, size=0.6, width=0.35) +
    scale_y_continuous('Ratio')+
    scale_x_discrete("Nº of Vehicles in SIMs") +
    ggtitle("Interest Dropped Ratios") +
    scale_colour_discrete(name  ="",
                            breaks=c("ISNFD", "ISVAL"),
                            labels=c("NFD Int\nDropped\nRatio\n", "\nVAL Int\nDropped\nRatio")) +
    scale_shape_discrete(name  ="",
                           breaks=c("ISNFD", "ISVAL"),
                            labels=c("NFD Int\nDropped\nRatio\n", "\nVAL Int\nDropped\nRatio"))

intDroppedRatiosPlot

################################3

dataRatiosPlot <- ggplot(data=dataRatios, aes(x=N_VEH, , y=MEAN, group=TYPE, colour=TYPE))+
    theme(axis.title = element_text(size = 14)) +
    theme(axis.text = element_text(size = 16)) +
    theme(plot.title = element_text(size = 16)) +
    theme(legend.text = element_text(size = 14)) +
    theme(legend.title = element_text(size = 14)) +
    geom_line(linetype=2, size=0.1) +
    geom_point(aes(shape=TYPE), size=4.5) +
    geom_text(aes(label=MEAN), vjust=-0.7, hjust=1.4, size=3)+
    geom_errorbar(aes(ymin=LB, ymax=HB), linetype=1, size=0.6, width=0.35) +
    scale_y_continuous('Ratio')+
    scale_x_discrete("Nº of Vehicles in SIMs") +
    ggtitle("Data Ratios") +
    scale_colour_discrete(name  ="",
                            breaks=c("DDR","VODp"),
                            labels=c("Data\nDelivery\nRatio\n","\nSystem\nOut Data\nRatio")) +
    scale_shape_discrete(name  ="",
                           breaks=c("DDR","VODp"),
                            labels=c("Data\nDelivery\nRatio\n","\nSystem\nOut Data\nRatio"))

dataRatiosPlot

dataDroppedRatiosPlot <- ggplot(data=dataDroppedRatios, aes(x=N_VEH, , y=MEAN, group=TYPE, colour=TYPE))+
    theme(axis.title = element_text(size = 14)) +
    theme(axis.text = element_text(size = 16)) +
    theme(plot.title = element_text(size = 16)) +
    theme(legend.text = element_text(size = 14)) +
    theme(legend.title = element_text(size = 14)) +
    geom_line(linetype=2, size=0.1) +
    geom_point(aes(shape=TYPE), size=4.5) +
    geom_text(aes(label=MEAN), vjust=-0.7, hjust=1.4, size=3)+
    geom_errorbar(aes(ymin=LB, ymax=HB), linetype=1, size=0.6, width=0.35) +
    scale_y_continuous('Ratio')+
    scale_x_discrete("Nº of Vehicles in SIMs") +
    ggtitle("Data Dropped Ratios") +
    scale_colour_discrete(name  ="",
                            breaks=c("DSNFD", "DSVAL"),
                            labels=c("NFD Data\nDropped\nRatio\n", "\nVAL Data\nDropped\nRatio")) +
    scale_shape_discrete(name  ="",
                           breaks=c("DSNFD", "DSVAL"),
                            labels=c("NFD Data\nDropped\nRatio\n", "\nVAL Data\nDropped\nRatio"))

dataDroppedRatiosPlot



#############################################

# GEO STORY #

geoStoryPlot <- ggplot(data=geoStory, aes(x=N_VEH, , y=MEAN, group=TYPE, colour=TYPE))+
    theme(axis.title = element_text(size = 14)) +
    theme(axis.text = element_text(size = 16)) +
    theme(plot.title = element_text(size = 16)) +
    theme(legend.text = element_text(size = 14)) +
    theme(legend.title = element_text(size = 14)) +
    geom_line(linetype=2, size=0.1) +
    geom_point(aes(shape=TYPE), size=4.5) +
    geom_text(aes(label=MEAN), vjust=-0.7, hjust=1.4, size=3)+
    geom_errorbar(aes(ymin=LB, ymax=HB), linetype=1, size=0.6, width=0.35) +
    scale_y_continuous('Nº Out Interest packets')+
    scale_x_discrete("Nº of Vehicles in SIMs") +
    ggtitle("Discovery Ratio") +
    scale_colour_discrete(name  ="",
                            breaks=c("DEST","DISC"),
                            labels=c("Out Int with\nKnown\nDestination\n","\nOut Int with\nUnknown\nDestination")) +
    scale_shape_discrete(name  ="",
                           breaks=c("DEST","DISC"),
                            labels=c("Out Int with\nKnown\nDestination\n","\nOut Int with\nUnknown\nDestination"))

geoStoryPlot


###############################

delayStoryPlot <- ggplot(data=delayStory, aes(x=N_VEH, , y=MEAN, group=TYPE, colour=TYPE))+
    theme(axis.title = element_text(size = 14)) +
    theme(axis.text = element_text(size = 16)) +
    theme(plot.title = element_text(size = 16)) +
    theme(legend.text = element_text(size = 14)) +
    theme(legend.title = element_text(size = 14)) +
    geom_line(linetype=2, size=0.1) +
    geom_point(aes(shape=TYPE), size=4.5) +
    geom_text(aes(label=MEAN), vjust=-0.7, hjust=1.4, size=3)+
    geom_errorbar(aes(ymin=LB, ymax=HB), linetype=1, size=0.6, width=0.35) +
    scale_y_continuous('Seconds')+
    scale_x_discrete("Nº of Vehicles in SIMs") +
    ggtitle("Delays") +
    scale_colour_discrete(name  ="",
                            breaks=c("FULLDELAY","LASTDELAY"),
                            labels=c("Full\nDelay\n","\nLast\nDelay")) +
    scale_shape_discrete(name  ="",
                           breaks=c("FULLDELAY","LASTDELAY"),
                          labels=c("Full\nDelay\n","\nLast\nDelay"))
delayStoryPlot

