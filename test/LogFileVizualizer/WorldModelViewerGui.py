# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'WorldModelViewer.ui'
#
# Created: Thu Dec  5 17:50:39 2013
#      by: PyQt4 UI code generator 4.10.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_worldModelViewer(object):
    def setupUi(self, worldModelViewer):
        worldModelViewer.setObjectName(_fromUtf8("worldModelViewer"))
        worldModelViewer.resize(800, 600)
        self.centralwidget = QtGui.QWidget(worldModelViewer)
        self.centralwidget.setObjectName(_fromUtf8("centralwidget"))
        self.verticalLayout = QtGui.QVBoxLayout(self.centralwidget)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.worldModelImage = QtGui.QLabel(self.centralwidget)
        self.worldModelImage.setObjectName(_fromUtf8("worldModelImage"))
        self.verticalLayout.addWidget(self.worldModelImage)
        worldModelViewer.setCentralWidget(self.centralwidget)

        self.retranslateUi(worldModelViewer)
        QtCore.QMetaObject.connectSlotsByName(worldModelViewer)

    def retranslateUi(self, worldModelViewer):
        worldModelViewer.setWindowTitle(_translate("worldModelViewer", "WorldModel Viewer", None))
        self.worldModelImage.setText(_translate("worldModelViewer", "worldmodel", None))

class Ui_ImageViewer(object):
    def setupUi(self, imageViewer):
        imageViewer.setObjectName(_fromUtf8("imageViewer"))
        imageViewer.resize(650, 500)
        self.verticalLayout = QtGui.QVBoxLayout(imageViewer)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.img = QtGui.QLabel(imageViewer)
        self.img.setObjectName(_fromUtf8("img"))
        self.verticalLayout.addWidget(self.img)

        self.retranslateUi(imageViewer)
        QtCore.QMetaObject.connectSlotsByName(imageViewer)

    def retranslateUi(self, imageViewer):
        imageViewer.setWindowTitle(_translate("imageViewer", "Image Viewer", None))
