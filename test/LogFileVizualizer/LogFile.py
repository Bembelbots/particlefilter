import sys
import time
import os

from Vision import *
from Definitions import *
from Constants import Events

class Message:
    def __init__(self, rawmessage):
        self.good = True
        try:
            (timestampMs, message) = rawmessage.split(" ", 1)
        except Exception,e :
            print "failed to parse line, error: %s" % str(e)
            print rawmessage
            self.good = False
            return
        
        try:
            self.timestampMs = int(timestampMs[1:-1])
            self.message = message 
        except Exception,e :
            #print "failed to parse line, error: %s" % str(e)
            print rawmessage
            self.good = False
            return

        self.visionResult = False
        self.robot = False
        self.image = False
        self.odo = False
        self.isTransform = False
        self.isNewTick = False
        self.measurement = False
        self.hypo = False
        self.debugPos = False
        self.tevent = False
        if self.message.startswith("VisionResult"):
            self.message = self.message.split(" ", 1)[1]
            vr = VisionResult()
            vr.setFromString(self.message)
            self.message = vr
            self.visionResult = True

        elif self.message.find(".bl2") != -1:
            for fn in self.message.split(" "):
                if fn.find(".bl2") != -1:
                    if fn.startswith("/"): # full path
                        self.message = fn
                    else:
                        self.message = fn[fn.find("images/"):]
                    break
            self.image = True

        elif self.message.startswith("WCS:"):
            self.message = self.message[5:]
            robot = Robot()
            robot.setFromString(self.message)
            self.message = robot
            self.robot = True

        elif self.message.startswith("Odometry:"):
            self.message = [float(x) for x in self.message[10:].split(";")]
            self.odo = True

        elif (self.message.startswith("top camera transform:") or self.message.startswith("bottom camera transform:")):
            if self.message.startswith("top camera transform:"):
                offset = 22
                self.cam = "top"
            else:
                offset = 25
                self.cam = "bottom"
            self.isTransform = True
            self.message = [float(x) for x in self.message[offset:-1].split(";")]

        elif self.message.startswith("enter new cognition step:"):
            self.message = int(self.message.split(" ")[-1])
            self.isNewTick = True

        elif self.message.startswith("Hypo:"):
            self.message = self.message[6:]
            robot = Robot()
            robot.setFromString(self.message)
            self.message = robot
            self.hypo = True

        elif self.message.startswith("Measurement"):
            try:
                vals = self.message[13:].split(";")
                if len(vals) == 1:
                    s = vals[0]
                else:
                    self.message = [int(vals[0][0])] + [float(vals[0][2:])] + [float(x) for x in vals[1:]]
                    s = "Robot id=0 s=2 team=3 t=-2 c=1.0 @ %f, %f, %frad GROUNDTRUTH: %f,%f,%f,Conf: 0" % \
                            (self.message[1], \
                            self.message[2], \
                            self.message[3], \
                            self.message[4], \
                            self.message[5], \
                            self.message[6])
                
                self.robotData = Robot()
                self.robotData.setFromString(s)
                if len(vals) == 1:
                    self.robotData.message.timestamp = -2
                    self.message = [0, self.robotData.posX, self.robotData.posY, self.robotData.alpha]
                
                self.measurement = True
                
            except Exception, e:
                print "measurement fail: %s" % str(e)
                print self.message
                self.good = False
        elif self.message.startswith("Debugpos"):
            try:
                vals = self.message[13:].split(";")
                if len(vals) == 1:
                    s = vals[0]
                else:
                    self.message = [int(vals[0][0])] + [float(vals[0][2:])] + [float(x) for x in vals[1:]]
                    s = "Robot id=0 s=2 team=3 t=-2 c=1.0 @ %f, %f, %frad GROUNDTRUTH: %f,%f,%f,Conf: 0" % \
                            (self.message[1], \
                            self.message[2], \
                            self.message[3], \
                            self.message[4], \
                            self.message[5], \
                            self.message[6])
                
                self.robotData = Robot()
                self.robotData.setFromString(s)
                if len(vals) == 1:
                    self.robotData.message.timestamp = -3
                    self.message = [0, self.robotData.posX, self.robotData.posY, self.robotData.alpha]
                
                self.measurement = True
                
            except Exception, e:
                print "measurement fail: %s" % str(e)
                print self.message
                self.good = False

        elif self.message.startswith("Emit"):
            self.eventId = int(self.message.split(": ")[1])
            #print "found event with id {}".format(Events.event_names[self.eventId])
            self.tevent = True

        else:
            print "whats this:"
            print self.message
            self.good = False

    def isEvent(self):
        return self.tevent

    def isRobot(self):
        return self.robot

    def isHypo(self):
        return self.hypo

    def isGood(self):
        return self.good

    def isVisionResult(self):
        return self.visionResult

    def isMeasurement(self):
        return self.measurement

    def isOdo(self):
        return self.odo

    def isImage(self):
        return self.image
        
    def getMessage(self):
        return self.message

class LogFile:
    def __init__(self, filename):
        f = open(filename, "r")
        self.content = f.read().split("\n")
        f.close()
        self.baseDir = os.path.split(filename)[0]

        self.steps = {}
        self.steps[0] = []

        self.events = {}

        self.parsed = self.parse()

        self.currentPos = 0
        
        self.finished = False

    def setStartTick(self, tick):
        self.currentPos = tick

    def parse(self):
        tick = 0
        for line in self.content:
            if not line:
                continue

            m = Message(line)
            if not m.isGood():
                continue
            if m.isNewTick:
                tick = m.message
                self.steps[tick] = []
                continue

            self.steps[tick].append(m)
            if m.isEvent():
                self.events[tick] = m.eventId

        self.keys = self.steps.keys()
        self.keys.sort()
        print "loaded %d datalines" % len(self.keys)
        self.currentPos = 0

        return True

    def getEvents(self):
        return self.events

    def isFinished(self):
        return self.finished
 
    def getCurrentPos(self):
        return self.currentPos
    
    def setCurrentTick(self, tick):
        if tick < 0:
            tick = 0
        if tick >= len(self.keys):
            tick = len(self.keys)-1

        self.currentPos = tick

    def getCurrentTick(self):
        return self.keys[self.currentPos]

    def getNextKey(self):
        key = self.keys[self.currentPos]
        if (self.currentPos + 1 == len(self.keys)):
            self.currentPos = 0
            self.finished = True
        else:
            self.currentPos = self.currentPos + 1
        
        return key

    def getNextTick(self):
        return self.steps[self.getNextKey()]   

from PyQt4 import QtCore, QtGui
class LogFilePlayer(QtCore.QThread):
    def __init__(self, filename):
        QtCore.QThread.__init__(self)
        self.filename = filename
        self.logfile = LogFile(filename)

        self.exiting = False
        self.last = 0
        self.timestamp = 0

        self.play = False
        self.waitCnt = 0
        self.waitTime = -1

    def pause(self):
        self.play = False

    def resume(self):
        self.play = True

    def step(self, steps=1):
        if steps == -1:
            self.logfile.setCurrentTick(self.logfile.getCurrentPos()-2)
        self.singleStep()

    def gotoPos(self, pos):
        ticktogo = pos["log"]
        self.logfile.setStartTick(ticktogo)
        self.singleStep()

    def singleStep(self):
        data = self.logfile.getNextTick()
        self.emit(QtCore.SIGNAL("newData"), data)

        now = self.last
        for d in data:
            now = d.timestampMs
            self.timestamp = now

        if self.last == 0:
            self.waitTime = -1
        else:
            self.waitTime = now-self.last
        
        self.last = now
        
    def run(self):
        while not self.exiting:
            if not self.play:
                time.sleep(0.1)
                continue

            self.singleStep()   
            
            if self.waitTime > 0:
                time.sleep(self.waitTime / 1000.0)
                print "sleep ", self.waitTime

            if self.logfile.isFinished():
                self.emit(QtCore.SIGNAL("newData"), [])
                print "no data left, pause"
                self.play = False
                self.logfile.finished = False

    def stop(self):
        self.__del__()

    def __del__(self):
        self.exiting = True

if __name__ == "__main__":

    lfp = LogFilePlayer(sys.argv[1])
    lfp.start()
    while 1:
        time.sleep(1)

