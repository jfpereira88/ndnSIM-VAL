args = commandArgs(trailingOnly=TRUE)

if(length(args) != 5) {
  stop("app-trace, rate-trace and val_pkts files needed, 5 arguments.", call.=FALSE)
}


appDt <- read.table(args[1], header=TRUE)
rateDt <- read.table(args[2], header=TRUE)
valDt <- read.table(args[3], header=FALSE)
sim <- args[4]
nVeh <- args[5]


colnames(valDt) <- c("Node", "Type", "Packets")

filename <- paste("results-sim-", sim, ".txt", sep = "")
#pdf(file=filename)


## app data
appDt$RetxCount <- appDt$RetxCount -1
appDtFullD <- subset(appDt, Type %in% c("FullDelay"))
appZeros <- subset(appDtFullD, DelayS == '0' | HopCount == '0')
appDtLastD <- subset(appDt, Type %in% c("LastDelay"))

#### Application values
fullDelayMean <- sum(appDtFullD$DelayS)/nrow(appDtFullD)
lastDelayMean <- sum(appDtLastD$DelayS)/nrow(appDtLastD)
selfServed <- nrow(appZeros)
appRtx <- sum(appDt$RetxCount)
####

## rate trace data
appFaces <- subset(rateDt, FaceDescr %in% c("appFace://"))
geoFaces <- subset(rateDt, FaceDescr %in% c("geoFace://"))
outGeofaces <- subset(geoFaces, Type %in% c("OutInterests"))
dicoveryFaces <- subset(outGeofaces, FaceId %in% c("256"))
knownFaces <- subset(outGeofaces, !FaceId %in% c("256"))

appInInt <- subset(appFaces, Type %in% c("InInterests"))   # received from appface
appOutInt <- subset(appFaces, Type %in% c("OutInterests")) #sent to appface
appInData <- subset(appFaces, Type %in% c("InData"))    # received from appface
appOutData <- subset(appFaces, Type %in% c("OutData"))  # sent to appface

#### AppFace values
nAppInInt <- sum(appInInt$PacketRaw)
nAppOutInt <- sum(appOutInt$PacketRaw)
nAppInData <- sum(appInData$PacketRaw)
nAppOutData <- sum(appOutData$PacketRaw)
satIntRacio <- nAppOutData/nAppInInt
meanSatIntRatio_Node <- nAppOutData/10
#####

nfdInInt <-  subset(geoFaces, Type %in% c("InInterests"))
nfdOutInt <-  subset(geoFaces, Type %in% c("OutInterests"))
nfdInData <- subset(geoFaces, Type %in% c("InData"))
nfdOutData <- subset(geoFaces, Type %in% c("OutData"))

#### NFD Values
nNfdInInt <-  sum(nfdInInt$PacketRaw)
nNfdOutInt <-  sum(nfdOutInt$PacketRaw)
nNfdInData <- sum(nfdInData$PacketRaw)
nNfdOutData <- sum(nfdOutData$PacketRaw)
nDicovery <- sum(dicoveryFaces$PacketRaw)
nKnown <- sum(knownFaces$PacketRaw)

## VAL data
valInInt <- subset(valDt, Type %in% c("InIntPkts"))
valOutInt <- subset(valDt, Type %in% c("OutIntPkts"))
valInData <- subset(valDt, Type %in% c("InDataPkts"))
valOutData <- subset(valDt, Type %in% c("OutDataPkts"))
valIntRtx <- subset(valDt, Type %in% c("RtxIntPkts"))
valDataRtx <- subset(valDt, Type %in% c("RtxDataPkts"))
valHLTtoRTX <- subset(valDt, Type %in% c("HLTtoRTX"))


#### VAL Values
nValInInt <- sum(valInInt$Packets)
nValOutInt <- sum(valOutInt$Packets)
nValInData <- sum(valInData$Packets)
nValOutData <- sum(valOutData$Packets)
nValIntRtx <- sum(valIntRtx$Packets)
nValDataRtx <- sum(valDataRtx$Packets)
nValHLTtoRTX <- sum(valHLTtoRTX$Packets)


line <- paste(sim, nVeh, nValHLTtoRTX, nValIntRtx, nValDataRtx, nValInInt, nValOutInt, nValOutInt/nValInInt, nValInData, nValOutData, nValOutData/nValInData,
                nNfdInInt, nNfdOutInt, nNfdOutInt/nNfdInInt, nNfdInData, nNfdOutData, nNfdOutData/nNfdInData, nDicovery, nKnown, nKnown/nNfdOutInt,
                nAppInInt, nAppOutInt ,nAppInData, nAppOutData, nAppOutData/nAppInData, (nAppOutData/nAppInData)-1, appRtx, fullDelayMean, lastDelayMean, 
                selfServed, satIntRacio, meanSatIntRatio_Node ,"\n", sep=",")
cat(line, file = filename)
