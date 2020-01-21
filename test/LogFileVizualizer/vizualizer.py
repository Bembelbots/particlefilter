# -*- coding: UTF-8 -*-

from PyQt4.QtGui import *
from PyQt4.QtCore import *
from PyQt4.QtSvg import *
import signal
import sys,os
from optparse import OptionParser

from Worldmodel import WorldModel
from Vision import VisionResult
from LogFile import LogFile, LogFilePlayer
import Constants
from WorldModelViewerGui import *
import time

NUM_OF_ROBOS = 5

import signal
signal.signal(signal.SIGINT, signal.SIG_DFL)

class SyncTime():
    def __init__(self):
        self.startTimeMs = self.getCurrentTimeMs()
    
    def getCurrentTimeMs(self):
        return int(time.time() * 1000)

    def getTime(self):
        return self.getCurrentTimeMs() - self.startTimeMs


class WorldModelViewer(QtGui.QMainWindow):
    def __init__(self, options, args):
        QtGui.QWidget.__init__(self)
        self.ui = Ui_worldModelViewer()
        self.ui.setupUi(self)
        self.rotatePixmap = False
        
        self.robos = [None] * NUM_OF_ROBOS
        self.event = []
        self.robos_time = [None] * NUM_OF_ROBOS
        self.balls = [None] * NUM_OF_ROBOS
        self.gcData = None
        self.visionResults = []
        self.measurements = []
        self.hypos = []
        self.positions = []
        self.allhypos = [[]]
        self.seeRobo = False

        self.worldModel = WorldModel(self.ui, int(options.fieldsize))
        self.ui.worldModelImage.resizeEvent = self.onResize

        self.options = options

        self.oldSize = (self.width(), self.height())
        # start as fullscreen 
        if options.fullscreen:
            self.setWindowState(Qt.WindowFullScreen) 
            resolution = QtGui.QDesktopWidget().screenGeometry()
            w = resolution.width()
            h = resolution.height()
        else:
            w = self.oldSize[0]
            h = self.oldSize[1]

        self.worldModel.updateImageDimension(w, h)
        self.time = SyncTime()

        self.timeout = 500

        print "replay data from log file %s" % options.logfile
        self.logfile = LogFilePlayer(options.logfile)
        self.logFileBase = os.path.split(options.logfile)[0]

        print "use keys to navigate:"
        print "spacebar: start / stop auto play"
        print "arrow-keys to navigate"
        print "r: reload currently loaded file"

        self.play = False
        self.logfile.start()
        QtCore.QObject.connect(self.logfile, QtCore.SIGNAL("newData"), self.addNewData)
        self.timeout = 50

        self.worldModelDrawer = QtCore.QTimer()
        QtCore.QObject.connect(self.worldModelDrawer, QtCore.SIGNAL("timeout()"), self.drawWorldmodel)
        self.worldModelDrawer.start(self.timeout)
        
    
    def togglePixmap(self):
        self.rotatePixmap = not self.rotatePixmap
        print self.rotatePixmap

    def drawWorldmodel(self):
        self.worldModel.resetPixmap()
        self.updateWorldmodel()
        self.worldModel.redraw(self.rotatePixmap)

    def updateWorldmodel(self):
    	if self.seeRobo:
            self.worldModel.addRobos(self.measurements)
            self.worldModel.addRobos(self.hypos)
            self.worldModel.addRobos(self.robos)
            self.worldModel.addVisionResults(self.balls)
            self.worldModel.addVisionResults(self.visionResults)
            #self.worldModel.addEvents(self.event)
    	
    def addNewData(self, data):
        self.visionResults = []
        self.measurements = []
        self.hypos = []
        self.seeRobo = False

        for d in data:
            if d.isVisionResult():
                if type(d.message.vtype) == type(int()):
                    d.message.vtype = d.message.getTypeById(d.message.vtype)
                #if vr.vtype == 5:
                #    res.write("%d,line,%f,%f,%f,%f\n" % (d.timestampMs, vr.rcs_x1, vr.rcs_y1, vr.rcs_x2, vr.rcs_y2))
                self.visionResults.append(d.getMessage())
                self.write("vision result: type(%s), ics(%d,%d), rcs(%0.2f,%0.2f)" % \
                        (d.getMessage().vtype, \
                        d.getMessage().ics_x1, \
                        d.getMessage().ics_y1, \
                        d.getMessage().rcs_x1, \
                        d.getMessage().rcs_y1))

            elif d.isRobot():
            	self.seeRobo = True
                self.write("WCS: %s" % d.getMessage())
                self.addNewRobot(d.getMessage())
                self.positions.append(d.getMessage())


            elif d.isMeasurement():
                self.measurements.append(d.robotData)
                self.write("Measurement: %s" % d.robotData)

            elif d.isHypo():
                self.hypos.append(d.getMessage())
                self.hypos[-1].message.timestamp = -1
                self.write("Hypo: %s" % d.message)
           
            else:
                self.write("%s" % d.getMessage())
        self.allhypos.append(self.hypos)
        self.drawWorldmodel()

    def eventIdtoEvent(self, id):
        return Constants.Events.event_names[id]

    def write(self, what):
        print "[%08d] %s" % (self.logfile.logfile.currentPos, what)
    
    def addNewVisionResult(self, vr):
        self.visionResults.append(vr)

    def addNewBall(self, ball):
        vr = VisionResult()
        vr.createFromBall(ball)
        self.balls[ball.message.sender] = vr

    def addNewRobot(self, robot):
        if robot.message.timestamp != -2:
            self.robos[robot.robotId] = robot
            self.robos_time[robot.robotId] = self.time.getTime()
        self.worldModel.myId = robot.robotId

    def onResize(self, event=None):
        self.worldModel.resize()

    def cleanup(self,a=None, b=None):
        if self.logfile is not None:
            print "cleanup: replay data"
            self.logfile.stop()
        sys.exit()

    def forward(self):
        self.logfile.step()

    def rewind(self):
        self.logfile.step(-1)

    def handleKeyPress(self, key):
        if key == Qt.Key_Q:
            self.cleanup()
        elif key == Qt.Key_Space:
            self.toggleAutoPlay()
        elif key == Qt.Key_PageUp:
            pass
        elif key == Qt.Key_PageDown:
            pass
        elif key == Qt.Key_Left:
            if self.logfile is not None:
                self.logfile.step(-1)
        elif key == Qt.Key_Right:
            if self.logfile is not None:
                self.logfile.step()
        elif key == Qt.Key_R: # reload
            if self.logfile is not None:
                self.logfile.stop()
                self.initLogFile()
                self.logfile.singleStep()
        else:
            print "key {} not implemented".format(key)

    def doPlay(self):
        self.logfile.resume()
        self.play = True
        #self.worldModelDrawer.start(self.timeout)


    def doPause(self):
        self.logfile.pause()
        self.play = False
        #self.worldModelDrawer.stop()


    def toggleAutoPlay(self):
        if self.logfile.play:
            self.doPause()
        else:
            self.doPlay()

    def event(self,event):
        if event.type()==QEvent.KeyPress:
            self.handleKeyPress(event.key())

        return QtGui.QWidget.event(self, event)

    def changeEvent(self, event):

        resolution = QtGui.QDesktopWidget().screenGeometry()
        QtGui.QWidget.changeEvent(self, event)

        if event.type() == QtCore.QEvent.WindowStateChange:
            if self.windowState() & QtCore.Qt.WindowMaximized:
                self.worldModel.updateImageDimension(resolution.width(), resolution.height())
            """elif event.oldState() & QtCore.Qt.WindowMaximized:
                print('changeEvent: Normal/Maximised/FullScreen')"""



if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("-l", "--logfile", help="the path to the logfile that should be vizualized")
    parser.add_option("-f", "--fullscreen", dest="fullscreen", action="store_true", default=False, help="start in fullscreen mode")
    parser.add_option("-s", "--fieldsize", dest="fieldsize", action="store", default=0, help="field Size (0=small, 1=spl12, 2=big)")
    
    (options, args) = parser.parse_args()
    print options
    app = QtGui.QApplication(sys.argv)
    wmv = WorldModelViewer(options, args)

    signal.signal(signal.SIGINT, wmv.cleanup)

    wmv.show()

    # Now we can start it.
    sys.exit(app.exec_())

