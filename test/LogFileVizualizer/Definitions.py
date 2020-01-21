# -*- coding: UTF-8 -*-

from struct import pack, unpack_from

SPL_STANDARD_MESSAGE_STRUCT_HEADER = "SPL "
SPL_STANDARD_MESSAGE_STRUCT_VERSION = 6
SPL_STANDARD_MESSAGE_DATA_SIZE = 780
SPL_STANDARD_MESSAGE_MAX_NUM_OF_PLAYERS = 5

SIZE_SPL_MESSAGE = 852
SIZE_ROBOT = 64
SIZE_BALL = 36
SENSOR_DATA_SIZE = 660

class Message:
    def __init__(self, data=None):
        self.timestamp = 0
        
        if data is not None:
            self.parseData(data)

    def parseData(self, data):
        vals = unpack_from("i", data)
        self.timestamp = vals[0]

    def __str__(self):
        return " (Message at %d) " % (self.timestamp)

class Ball:
    def __init__(self, data=None):
        self.message = Message()
        self.id = -1
        self.posX = 0.0
        self.posY = 0.0
        self.confidence = 0.0
        self.motionX = 0.0
        self.motionY = 0.0
        self.motionConfidence = 0.0
        self.localConfidence = 0.0

        if data is not None:
            self.parseData(data)

    def parseData(self, data):
            self.message = Message(data[:4])
            vals = unpack_from("i7f", data[4:])
            self.id = vals[0]
            self.posX = vals[1]
            self.posY = vals[2]
            self.confidence = vals[3]
            self.motionX = vals[4]
            self.motionY = vals[5]
            self.motionConfidence = vals[6]
            self.localConfidence = vals[7]

    def __str__(self):
        msg = "Ball at (%0.2f, %0.2f) @ %0.2f with motion (%0.2f, %0.2f) @ %0.2f" % (self.posX, self.posY, self.confidence, self.motionX, self.motionY, self.motionConfidence)
        return msg + str(self.message)

class Robot:
    def __init__(self, data=None):
        if data is not None:
            self.message = Message(data[:4])
            vals = unpack_from("2if2i4fi4fi", data[4:])
            self.robotId = vals[0]
            self.role = vals[1]
            self.fallenSince = vals[2]
            self.active = vals[3]
            try:
                self.alphaDeg = int(vals[4])
                self.alpha = float(vals[5])
            except Exception, e:
                self.alphaDeg = 0
                self.alpha = 0.0
                print "warning: failed to parse angle: %s!" % str(e)
            self.posX = vals[6]
            self.posY = vals[7]
            self.confidence = vals[8]
            self.GTposAlphaDeg = vals[9]
            self.GTposAlpha = vals[10]
            self.GTposX = vals[11]
            self.GTposY = vals[12]
            self.GTconfidence = vals[13]
            self.GTtimestamp = vals[14]
        else:
            self.message = Message()
            self.robotId = -1
            self.role = -1
            self.fallenSince = 0.0
            self.active = 0
            self.alphaDeg = 0
            self.alpha = 0.0
            self.posX = 0.0
            self.posY = 0.0
            self.confidence = 0.0
            self.GTposAlphaDeg = 0
            self.GTposAlpha = 0.0
            self.GTposX = 0.0
            self.GTposY = 0.0
            self.GTconfidence = 0.0
            self.GTtimestamp = 0

    def setFromString(self, s):
        # new Robot id=0 t=3179 c=1 @ -3.21, -3, 1.568(rad)rad GROUNDTRUTH: 0,0,0(rad),Conf: 0
        # Robot id=2 s=2 team=3 t=6130 c=0 @ 2.14, -2.47, 2.04rad GROUNDTRUTH: 0,0,0,Conf: 0
        v = s.split(" ")
        #new Robot
        if v[2][0] == "t":
            self.message = Message()
            self.message.timestamp = int(v[2][2:])
            self.robotId = int(v[1][3:])
            self.posX = float(v[5][:-1])
            self.posY = float(v[6][:-1])
            self.alpha = float(v[7][:-8])
            gt = v[9].split(",")
            self.GTposX = float(gt[0])
            self.GTposY = float(gt[1])
            self.GTposAlpha = float(gt[2][:-5])
            self.confidence = float(v[3][2:])
        #old Robot
        else:          
            self.message.timestamp = int(v[4][2:])
            self.robotId = int(v[1][3:])
            self.posX = float(v[7][:-1])
            self.posY = float(v[8][:-1])
            self.alpha = float(v[9][:-3])
            gt = v[11].split(",")
            self.GTposX = float(gt[0])
            self.GTposY = float(gt[1])
            self.GTposAlpha = float(gt[2])
            self.confidence = float(v[5][2:])


    def c_str(self):
        msg = "Robot id=%d s=%d team=%d t=%d c=%f @ %f, %f, %frad GROUNDTRUTH: %f,%f,%f,Conf: %f" % \
                (self.robotId, \
                self.message.sender, \
                self.message.teamNumber, \
                self.message.timestamp, \
                self.confidence, \
                self.posX, \
                self.posY, \
                self.alpha, \
                self.GTposX, \
                self.GTposY, \
                self.GTposAlpha, \
                self.GTconfidence)
        return msg

    def __str__(self):
        msg = "Robot #%d (%0.2f, %0.2f) @ %0.2f with conf %0.2f, (%0.2f, %0.2f) @ %0.2f " % \
                (self.robotId, \
                self.posX, \
                self.posY, \
                self.alpha, \
                self.confidence,\
                self.GTposX, \
                self.GTposY, \
                self.GTposAlpha)

        return msg + str(self.message)

class SPLStandardMessage:
    def __init__(self, data):
        if len(data) < SIZE_SPL_MESSAGE:
            print "size does not match spl standard message format!"
            return

        self.bembelbotsMessage = False

        raw = data[:(SIZE_SPL_MESSAGE-SPL_STANDARD_MESSAGE_DATA_SIZE)]
        #print len(raw)

        vals = unpack_from("4sBbbb3f2f2ff2f2f5bbhhbbh", raw)
        #print vals

        self.header = vals[0]

        if self.header != SPL_STANDARD_MESSAGE_STRUCT_HEADER:
            print "packet does not look like a spl standard message!"
            return

        self.version = vals[1]
        if self.version != SPL_STANDARD_MESSAGE_STRUCT_VERSION:
            print "spl message has wrong version!"
            return

        self.playerNum = vals[2]

        self.team = vals[3]
        self.fallen = vals[4]

        self.posX = vals[5]
        self.posY = vals[6]
        self.alpha = vals[7]

        self.walkX = vals[8]
        self.walkY = vals[9]

        self.shootX = vals[10]
        self.shootY = vals[11]

        self.ballAge = vals[12]

        self.ballX = vals[13]
        self.ballY = vals[14]

        self.motionX = vals[15]
        self.motionY = vals[16]

        self.suggestions = vals[17:17+SPL_STANDARD_MESSAGE_MAX_NUM_OF_PLAYERS]
        self.intention = vals[22]
        self.walkSpeed = vals[23]
        self.kickDist = vals[24]
        self.posConfidence = vals[25]
        self.sideConfidence = vals[26]
        self.dataSize = vals[27]
        
        #print
        #print "protocol version: %d" % self.version
        #print "player number: %d" % self.playerNum
        #print "team number %d" % self.team

        #print "fallen down? %d" % self.fallen
        #print "pos (%f,%f) @ %f" % (self.posX, self.posY, self.alpha)
        #print "ball age %d " % self.ballAge
        #print "ball (%f, %f)" % (self.ballX, self.ballY)
        #print "motion (%f, %f)" % (self.motionX, self.motionY)
        #print "additional data size %d" % self.dataSize
        #print

        blob = data[len(raw)-2:]
        if self.dataSize > 3 and blob[0] == 'B' and blob[1] == 'B' and blob[2] == ' ':
            try:
                self.bbRobot = Robot(blob[3:3+SIZE_ROBOT])
                self.bbBall = Ball(blob[3+SIZE_ROBOT:])
                self.bembelbotsMessage = True
            except Exception, e:
                print "failed to parse additional info\n%s" % str(e)


class SensorData():
    def __init__(self, data):
        if len(data) != SENSOR_DATA_SIZE:
            return
        
        lenName = 24
        lenSensors = 144
        lenEulers = 6
        all = unpack_from("24c144f6f6f2i", data)
        robotNameD = all[0:lenName]
        robotName = ""
        for i in robotNameD:
            if ord(i) == 0:
                break
            else:
                robotName += i
        self.robotName = robotName

        p = lenName
        self.sensors = all[p:p+lenSensors]

        p += lenSensors
        self.eulersTop = all[p:p+lenEulers]
        
        p += lenEulers
        self.eulersBottom = all[p:p+lenEulers]

        self.timestamp = all[-2]
        self.tick = all[-1]

