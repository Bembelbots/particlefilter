from struct import *
from StringIO import StringIO

class VisionResult():
    def __init__(self, data = None):
        self.types = ["nothing", "field", "ball", "rect", "goal", "line", "lcrossing","tcrossing","xcrossing", "penalty", "circle", "face", "scanpoint", "robot"]

        self.initValues()
        if data is None:
            return

        unpackData = unpack_from("8i8f2if", data)
        self.vtype = self.setType(unpackData[0])
        self.timestamp = unpackData[1]
        self.ics_x1 = unpackData[2]
        self.ics_y1 = unpackData[3]
        self.ics_x2 = unpackData[4]
        self.ics_y2 = unpackData[5]
        self.ics_width = unpackData[6]
        self.ics_height = unpackData[7]
        self.ics_confidence = unpackData[8]
        self.rcs_x1 = unpackData[9]
        self.rcs_y1 = unpackData[10]
        self.rcs_x2 = unpackData[11]
        self.rcs_y2 = unpackData[12]
        self.rcs_alpha = unpackData[13]
        self.rcs_distance = unpackData[14]
        self.rcs_confidence = unpackData[15]
        self.camera = unpackData[16]
        self.extra_int = unpackData[17]
        self.extra_float = unpackData[18]

        # added for ball motion
        self.ballMotionX = None
        self.ballMotionY = None
    
    def setFromString(self, vrs):
        x = vrs.split(";")

        self.vtype = int(x[0])
        self.timestamp = int(x[1])
        self.ics_x1 = int(x[2])
        self.ics_y1 = int(x[3])
        self.ics_x2 = int(x[4])
        self.ics_y2 = int(x[5])
        self.ics_width = int(x[6])
        self.ics_height = int(x[7])
        self.ics_confidence = float(x[8])
        self.rcs_x1 = float(x[9])
        self.rcs_y1 = float(x[10])
        self.rcs_x2 = float(x[11])
        self.rcs_y2 = float(x[12])
        self.rcs_alpha = float(x[13])
        self.rcs_distance = float(x[14])
        self.rcs_confidence = float(x[15])
        self.camera = int(x[16])
        self.extra_int = int(x[17])
        self.extra_float = float(x[18])
    
    def setType(self, t):
        return self.types[t]

    def getType(self, name):
        return self.types.index(name)

    def getTypeById(self, vid):
        return self.types[vid]

    def initValues(self):
        self.vtype = None
        self.timestamp = None
        self.ics_x1 = None
        self.ics_y1 = None
        self.ics_x2 = None
        self.ics_y2 = None
        self.ics_width = None
        self.ics_height = None
        self.ics_confidence = None
        self.rcs_x1 = None
        self.rcs_y1 = None
        self.rcs_x2 = None
        self.rcs_y2 = None
        self.rcs_alpha = None
        self.rcs_distance = None
        self.rcs_confidence = None
        self.camera = None
        self.extra_int = None
        self.extra_float = None

    def createFromBall(self, ball):
        """
        create vision result from received ball (Worldmodel)
        """
        self.vtype = "ball"
        self.rcs_x1 = ball.posX
        self.rcs_y1 = ball.posY
        self.ballMotionX = ball.motionX
        self.ballMotionY = ball.motionY

    def __str__(self):
        msg = ""
        msg += "vtype:          %s\n" % self.vtype
        msg += "timestamp:      %d\n" % self.timestamp
        msg += "ics_x1:         %d\n" % self.ics_x1
        msg += "ics_y1:         %d\n" % self.ics_y1
        msg += "ics_x2:         %d\n" % self.ics_x2
        msg += "ics_y2:         %d\n" % self.ics_y2
        msg += "ics_width:      %d\n" % self.ics_width
        msg += "ics_height:     %d\n" % self.ics_height
        msg += "ics_confidence: %0.2f\n" % self.ics_confidence
        msg += "rcs_x1:         %0.2f\n" % self.rcs_x1
        msg += "rcs_y1:         %0.2f\n" % self.rcs_y1
        msg += "rcs_x2:         %0.2f\n" % self.rcs_x2
        msg += "rcs_y2:         %0.2f\n" % self.rcs_y2
        msg += "rcs_alpha:      %0.2f\n" % self.rcs_alpha
        msg += "rcs_distance:   %0.2f\n" % self.rcs_distance
        msg += "rcs_confidence: %0.2f\n" % self.rcs_confidence
        msg += "camera:         %d\n" % self.camera
        msg += "extra_int:      %d\n" % self.extra_int
        msg += "extra_float:    %0.2f\n" % self.extra_float
        return msg

    def c_str(self):
        vals = [self.getType(self.vtype), self.timestamp, self.ics_x1, self.ics_y1, self.ics_x2, self.ics_y2, self.ics_width, self.ics_height, self.ics_confidence, self.rcs_x1, self.rcs_y1, self.rcs_x2, self.rcs_y2, self.rcs_alpha, self.rcs_distance, self.rcs_confidence, self.camera, self.extra_int, self.extra_float]
        s = ""
        for v in vals:
            s += str(v) + ";"
        return s[:-1]