# -*- coding: UTF-8 -*-

from PyQt4.QtGui import *
from PyQt4.QtCore import *
from PyQt4.QtSvg import *


#include <QPoint>
from Constants import *

import math
import time
timeMeasure = lambda: int(round(time.time() * 1000))
from thread import allocate_lock
painterLock = allocate_lock()

class WorldModel():
    def __init__(self, ui, fieldSize=FieldSize.SPL):
        self.fieldSize = -1
        self.updateFieldSize(fieldSize) # default size
        self.clearAfterPaint = True

        self.ui = ui
        self.painter = None
        self.painterReady = True
        self.painterStart = 0

        self.myId = None
        self.myPosX = 0
        self.myPosY = 0
        self.myPosAlpha = 0
        self.GTposX = 0
        self.GTposY = 0
        self.GTposAlpha =0

        self.robotPosition = (0,0)
        # prepare image
        self.updateImageDimension()
        self.resize()

        #self.resetPixmap()

    def setClearAfterPaint(self, value):
        self.clearAfterPaint = value

    def updateFieldSize(self, size):
        if size == self.fieldSize:
            return

        if size == FieldSize.HTWK:
            print "set field size to HTWK field"
            self.fieldSize = FieldSize.HTWK
            self.scale = 1.5
            self.image = "images/playing_field_640x468-htwk.png"
            self.qImage = QImage(self.image)

        elif size == FieldSize.SPL:
            print "set field size to SPL field"
            self.fieldSize = FieldSize.SPL
            self.scale = 1.5
            self.image = "images/playing_field_640x468-spl13.png"
            self.qImage = QImage(self.image)

        elif size == FieldSize.JRL:
            print "set field size to JRL field"
            self.fieldSize = FieldSize.JRL
            self.scale = 1.0
            self.image = "images/playing_field_640x468_yellow.png"
            self.qImage = QImage(self.image)

        else:
            print "try to set unknown field size, do nothing!"
    
        
    def updateImageDimension(self, w=0, h=0):

        if w == 0:
            self.imageWidth = self.ui.worldModelImage.width()-20
        else:
            self.imageWidth = w

        if h == 0:
            self.imageHeight = self.ui.worldModelImage.height()-10
        else:
            self.imageHeight = h

        self.updateScalingFactor()

    def getPixmapSize(self):
        pix = self.ui.worldModelImage.pixmap()
        if pix is not None:
            return pix.size()
        else:
            return QSize(0,0)

    def getPixmapWidth(self):
        return self.getPixmapSize().width()

    def getPixmapHeight(self):
        return self.getPixmapSize().height()

    def updateScalingFactor(self):
        # relation of pixel and m, 1m = 88.88 px in x
        if self.fieldSize == FieldSize.SPL:
            self.m2px = float(self.getPixmapWidth()) / 10.747
            self.m2py = float(self.getPixmapHeight()) / 7.844
        else:
            self.m2px = (float(self.getPixmapWidth()) / 7.37) / self.scale
            self.m2py = (float(self.getPixmapHeight()) / 5.379) / self.scale

    def mToPx(self, (x,y)):
        return x*self.m2px, y*self.m2py

    def WCStoICS(self, pos):
        """
        converts WCS coordinates to our image coordinates
        input parameters is a tuple (x,y). 
        """
        nx,ny = self.mToPx(pos)
        newX = nx + int(self.getPixmapWidth() / 2)
        newY = int(self.getPixmapHeight() / 2) - ny
        return int(newX), int(newY)

    def resetPixmap(self):
        painterLock.acquire()
        if not self.painterReady:
            print "drop paint request, last paint was not finished, yet!"
            return
        self.painterReady = False
        painterLock.release()

        self.painterStart = timeMeasure()
        self.pixmap = QPixmap.fromImage(self.qImage)
        self.painter = QPainter(self.pixmap)
        self.drawLegend()

    def resize(self, w=0):
        self.qImage = QImage(self.image)
        if w == 0:
            w = self.imageWidth
        self.qImage = self.qImage.scaledToWidth(w)
        self.pixmap = QPixmap.fromImage(self.qImage)
        self.redraw()

    def redraw(self, rotate=False):
        try:
            if self.painter is not None and self.clearAfterPaint:
                del self.painter
                self.painter = None
        except Exception,e:
            print "upps %s" % str(e)
    
        self.updateImageDimension()

        #pix = QPixmap.fromImage(self.qImage)
        if rotate:
            transform = QTransform().rotate(180)
            self.pixmap = self.pixmap.transformed(transform)

        self.ui.worldModelImage.setPixmap(self.pixmap)
        self.ui.worldModelImage.repaint()
        #print "paint consumes %d ms" % (timeMeasure() - self.painterStart)
        painterLock.acquire()
        self.painterReady = True
        painterLock.release()

    def drawLegend(self):
        def drawRobot(x,y,color, text):
            pen.setColor(color)
            pen.setWidth(7)
            self.painter.setPen(pen)
            self.painter.drawPoint(x, y)
            pen.setWidth(5)
            self.painter.setPen(pen)
            self.painter.drawLine(x+5, y,x+10,y)
            pen.setWidth(1)
            self.painter.setPen(pen)
            self.painter.drawText(x+15, y, 100, 20, Qt.AlignLeft, text)

        color = QColor()   
        pen = QPen()
        #VisionResults
        pen.setWidth(5)
        color.setRgb(0, 0, 255, 255) 
        pen.setColor(color)
        self.painter.setPen(pen)
        self.painter.drawLine(5, 5, 15, 5)
        font = QFont("Arial", 7)
        self.painter.setFont(font)
        self.painter.drawText(20, 0, 100, 20, Qt.AlignLeft, "VisionResults")
        #Estimated Pose
        color.setRgb(255, 0, 0)
        drawRobot(5,20,color, "Estimated Pose")
        #GT Pose
        color.setRgb(10, 100, 30) 
        drawRobot(5,40,color, "GT Pose")
        #Partikel
        color.setRgb(0,0, 0, 255) 
        drawRobot(5,60,color, "Partikel")
        #PoseHypos
        color.setRgb(255, 64, 255) 
        drawRobot(6,80,color, "Pose Hypotheses")


    def createPenFromColor(self, color):
        pen = QPen()
        pen.setColor(color)
        return pen


    def addEvents(self, events):
        if not len(events):
            return

        evs = list(events)
        if not len(evs):
            return 

        ev = Events()
        state = None
        state_events = ["EV_STATE_INITIAL", "EV_STATE_READY", "EV_STATE_SET", "EV_STATE_PLAYING", "EV_STATE_FINISHED"]
        state_event_ids = [ev.getId(x) for x in state_events]
        #for event in evs:
        #    if event in state_event_ids:
        #        state = Events.event_names[event].replace("EV_STATE_", "")
        #        evs.remove(event)

        w = self.getPixmapWidth()
        h = self.getPixmapHeight()
        font = QFont("Arial", w / 80)
        self.painter.setFont(font)

        #if state is not None:
        #    self.painter.drawText(0+w/30, 0+h/30, w, h / 15, Qt.AlignLeft, "Current State: {}".format(state))

        if not len(evs):
            return 
        lastEvent = Events.event_names[evs[-1]]
        self.painter.drawText(0+w/30, 0+3*(h/30), w, h / 15, Qt.AlignLeft, "Last Event: {}".format(lastEvent))


    def addRobos(self, robos):
        for r in robos:
            if r is not None:
                self.addRobo(r)


    def addRobo(self, robo):
        if robo.robotId == self.myId:
            self.myPosX = robo.posX
            self.myPosY = robo.posY
            self.myPosAlpha = float(robo.alpha)
            self.myGTposX = robo.GTposX
            self.myGTposY = robo.GTposY
            self.myGTposAlpha = float(robo.GTposAlpha)

        if self.painter == None:
            print "no painter ready so far"
            self.resetPixmap()
            return 

        def setPen(c, width, style = None):
            color = QColor()
            if len(c) == 4:
                color.setRgb(c[0],c[1],c[2],c[3])
            else:
                color.setRgb(c[0],c[1],c[2])
            pen = QPen()
            pen.setColor(color)
            if style is not None:
                pen.setCapStyle(style)
            pen.setWidth(width)
            self.painter.setPen(pen)

        isHypo = False
        isMeasurement = False
        isDebugpos = False
        if robo.message.timestamp == -1:
            isHypo = True
        elif robo.message.timestamp == -2:
            isMeasurement = True
        elif robo.message.timestamp == -3:
            isDebugpos = True

        (px, py) = self.WCStoICS((robo.posX, robo.posY))
        color = (255, 0, 0)   

        if isHypo:
            v = min(255,100+int(robo.confidence*225))
            color = (0,0,0,v)
        if isMeasurement:
            color = (255, 64, 255)
        if isDebugpos:
            color = (110, 70, 120)

        # convert to ICS
        setPen(color, width=max(1,self.ui.worldModelImage.width()/100))
        self.painter.drawPoint(px, py)
        
        setPen(color, width=max(1,self.ui.worldModelImage.width() / 200))
        
        # draw angle
        alpha = float(robo.alpha)

        pxt = int(px + (math.cos(alpha) * self.ui.worldModelImage.width() / 40))
        pyt = int(py - (math.sin(alpha) * self.ui.worldModelImage.width() / 40))
        setPen(color, width=self.ui.worldModelImage.width() / 250)
        self.painter.drawLine(px, py, pxt, pyt)

        #draw Groundtruth
        if robo.GTposX != 0 and robo.GTposY != 0 and not isHypo and not isMeasurement and not isDebugpos:
            setPen((10, 100, 30), width=1)
            (gtx, gty) = self.WCStoICS((robo.GTposX, robo.GTposY))
            self.painter.drawLine(px, py, gtx, gty)
            setPen((10, 100, 30), width=self.ui.worldModelImage.width() / 100)
            self.painter.drawPoint(gtx, gty)
            setPen((10, 100, 30), width=self.ui.worldModelImage.width() / 250)
            gtxp = int(gtx + (math.cos(robo.GTposAlpha) * self.ui.worldModelImage.width() / 40))
            gtyp = int(gty - (math.sin(robo.GTposAlpha) * self.ui.worldModelImage.width() / 40))
            self.painter.drawLine(gtx, gty, gtxp, gtyp)

        #print robot id
        if not isHypo and not isMeasurement :
            font = QFont("Arial",self.ui.worldModelImage.width() / 60)
            self.painter.setFont(font)
            self.painter.drawText(px - self.ui.worldModelImage.width() / 35, py - self.ui.worldModelImage.width() / 35, "(%d)" % (robo.robotId + 1)) 


    def rotate2D(self, x, y, px, py, alpha):
        nx = x + (math.cos(-alpha) * px) + (math.sin(-alpha) * py)
        ny = y - (math.sin(-alpha) * px) + (math.cos(-alpha) * py)
        return (nx, ny)

    def addVisionResults(self, vrs):
        for vr in vrs:
            if vr is not None:
                self.addVisionResult(vr)

    def addVisionResult(self, vr):
        def setPen(c, width, style = None): 
            color = QColor()
            color.setRgb(c[0],c[1],c[2],c[3])
            pen = QPen()
            pen.setColor(color)
            if style is not None:
                pen.setCapStyle(style)
            pen.setWidth(width)
            self.painter.setPen(pen)

        #myPos=[self.myPosX, self.myPosY, self.myPosAlpha]
        #if ((self.GTposX != 0) or (self.GTposY != 0) or (self.GTposAlpha != 0)):
        myPos=[self.myGTposX, self.myGTposY, self.myGTposAlpha]
        (px,py) = self.WCStoICS(self.rotate2D(myPos[0], myPos[1], vr.rcs_x1, vr.rcs_y1, myPos[2]))
        (px2,py2) = self.WCStoICS(self.rotate2D(myPos[0], myPos[1], vr.rcs_x2, vr.rcs_y2, myPos[2]))
        if vr.vtype == "ball" or vr.vtype == "goal" or vr.vtype == "robot":
            if vr.vtype == "ball":
                setPen((255, 0, 0,255), self.imageWidth/60, Qt.RoundCap)
            elif vr.vtype == "goal":
                setPen((255, 255, 0,255), self.imageWidth/60, Qt.RoundCap)
            elif vr.vtype == "robot":
                setPen((0, 0, 0,255), self.imageWidth/60, Qt.RoundCap)
            self.painter.drawPoint(int(px), int(py))

        elif vr.vtype == "line":
            setPen((0, 0, 255, 255), max(1, self.imageWidth / 240))
            self.painter.drawLine(px, py, px2, py2)

        elif vr.vtype == "lcrossing" or vr.vtype == "tcrossing" or vr.vtype == "xcrossing" :
            setPen((0, 0, 255, 255), self.imageWidth/60,Qt.RoundCap)
            self.painter.drawPoint(px, py)
            
            alpha = vr.extra_float + myPos[2]
            pxt = int(px + (math.cos(alpha) * self.ui.worldModelImage.width() / 20))
            pyt = int(py - (math.sin(alpha) * self.ui.worldModelImage.width() / 20))
            setPen((0, 0, 255, 255), self.imageWidth/140,Qt.RoundCap)
            self.painter.drawLine(px, py, pxt, pyt)
            
            font = QFont("Arial", self.ui.worldModelImage.width() / 60)
            self.painter.setFont(font)
            t = "%s" % str(vr.extra_int)
            if vr.vtype == "lcrossing":
                t = "L"
            elif vr.vtype == "tcrossing":
                t = "T"
            elif vr.vtype == "xcrossing":
                t = "X"
            (tx,ty) = self.rotate2D(px,py,self.ui.worldModelImage.width()/55, 1, myPos[2])
            self.painter.drawText(tx, ty, t)

        elif vr.vtype == "circle":
            setPen((0, 0, 255, 255), self.imageWidth/60,Qt.RoundCap)
            self.painter.drawPoint(px, py)
            
            setPen((0, 0, 255, 255), self.imageWidth/180,Qt.RoundCap)
            d = 1.2
            w,h = self.mToPx((d, d)) # please add diameter of center circle here
            self.painter.drawArc(px-(w/2), py-(h/2), w, h, 0, 5760)

        else:
            #print str(vr)
            pass