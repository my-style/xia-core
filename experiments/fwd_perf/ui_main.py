# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'main.ui'
#
# Created: Sun Nov  6 14:18:09 2011
#      by: PyQt4 UI code generator 4.8.5
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_Main(object):
    def setupUi(self, Main):
        Main.setObjectName(_fromUtf8("Main"))
        Main.resize(731, 684)
        Main.setWindowTitle(QtGui.QApplication.translate("Main", "XIA High-Speed Software Router Demo", None, QtGui.QApplication.UnicodeUTF8))
        Main.setModal(False)
        self.label = QtGui.QLabel(Main)
        self.label.setGeometry(QtCore.QRect(10, 10, 171, 17))
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label.sizePolicy().hasHeightForWidth())
        self.label.setSizePolicy(sizePolicy)
        self.label.setText(QtGui.QApplication.translate("Main", "Destination XIP Address", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setScaledContents(False)
        self.label.setAlignment(QtCore.Qt.AlignCenter)
        self.label.setObjectName(_fromUtf8("label"))
        self.label_2 = QtGui.QLabel(Main)
        self.label_2.setGeometry(QtCore.QRect(200, 10, 341, 20))
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_2.sizePolicy().hasHeightForWidth())
        self.label_2.setSizePolicy(sizePolicy)
        self.label_2.setFrameShadow(QtGui.QFrame.Plain)
        self.label_2.setText(QtGui.QApplication.translate("Main", "Packet Header", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setScaledContents(False)
        self.label_2.setAlignment(QtCore.Qt.AlignCenter)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.groupBox = QtGui.QGroupBox(Main)
        self.groupBox.setGeometry(QtCore.QRect(560, 30, 161, 101))
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.groupBox.sizePolicy().hasHeightForWidth())
        self.groupBox.setSizePolicy(sizePolicy)
        self.groupBox.setTitle(_fromUtf8(""))
        self.groupBox.setObjectName(_fromUtf8("groupBox"))
        self.radioButton_FB_0 = QtGui.QRadioButton(self.groupBox)
        self.radioButton_FB_0.setGeometry(QtCore.QRect(20, 20, 114, 22))
        self.radioButton_FB_0.setText(QtGui.QApplication.translate("Main", "No fallbacks", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_FB_0.setChecked(True)
        self.radioButton_FB_0.setObjectName(_fromUtf8("radioButton_FB_0"))
        self.radioButton_FB_1 = QtGui.QRadioButton(self.groupBox)
        self.radioButton_FB_1.setGeometry(QtCore.QRect(20, 40, 114, 22))
        self.radioButton_FB_1.setText(QtGui.QApplication.translate("Main", "1 fallback", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_FB_1.setObjectName(_fromUtf8("radioButton_FB_1"))
        self.radioButton_FB_2 = QtGui.QRadioButton(self.groupBox)
        self.radioButton_FB_2.setGeometry(QtCore.QRect(20, 60, 114, 22))
        self.radioButton_FB_2.setText(QtGui.QApplication.translate("Main", "2 fallbacks", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_FB_2.setObjectName(_fromUtf8("radioButton_FB_2"))
        self.radioButton_FB_3 = QtGui.QRadioButton(self.groupBox)
        self.radioButton_FB_3.setGeometry(QtCore.QRect(20, 80, 114, 22))
        self.radioButton_FB_3.setText(QtGui.QApplication.translate("Main", "3 fallbacks", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_FB_3.setObjectName(_fromUtf8("radioButton_FB_3"))
        self.label_3 = QtGui.QLabel(self.groupBox)
        self.label_3.setGeometry(QtCore.QRect(10, 0, 201, 21))
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_3.sizePolicy().hasHeightForWidth())
        self.label_3.setSizePolicy(sizePolicy)
        self.label_3.setText(QtGui.QApplication.translate("Main", "Number of fallbacks", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setScaledContents(False)
        self.label_3.setWordWrap(True)
        self.label_3.setObjectName(_fromUtf8("label_3"))
        self.groupBox_2 = QtGui.QGroupBox(Main)
        self.groupBox_2.setGeometry(QtCore.QRect(560, 150, 161, 141))
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.groupBox_2.sizePolicy().hasHeightForWidth())
        self.groupBox_2.setSizePolicy(sizePolicy)
        self.groupBox_2.setTitle(_fromUtf8(""))
        self.groupBox_2.setObjectName(_fromUtf8("groupBox_2"))
        self.radioButton_PS_128 = QtGui.QRadioButton(self.groupBox_2)
        self.radioButton_PS_128.setGeometry(QtCore.QRect(20, 20, 114, 22))
        self.radioButton_PS_128.setText(QtGui.QApplication.translate("Main", "128 bytes", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_PS_128.setChecked(True)
        self.radioButton_PS_128.setObjectName(_fromUtf8("radioButton_PS_128"))
        self.radioButton_PS_256 = QtGui.QRadioButton(self.groupBox_2)
        self.radioButton_PS_256.setGeometry(QtCore.QRect(20, 60, 114, 22))
        self.radioButton_PS_256.setText(QtGui.QApplication.translate("Main", "256 bytes", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_PS_256.setObjectName(_fromUtf8("radioButton_PS_256"))
        self.radioButton_PS_576 = QtGui.QRadioButton(self.groupBox_2)
        self.radioButton_PS_576.setGeometry(QtCore.QRect(20, 80, 114, 22))
        self.radioButton_PS_576.setText(QtGui.QApplication.translate("Main", "576 bytes", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_PS_576.setObjectName(_fromUtf8("radioButton_PS_576"))
        self.radioButton_PS_1088 = QtGui.QRadioButton(self.groupBox_2)
        self.radioButton_PS_1088.setGeometry(QtCore.QRect(20, 100, 114, 22))
        self.radioButton_PS_1088.setText(QtGui.QApplication.translate("Main", "1088 bytes", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_PS_1088.setObjectName(_fromUtf8("radioButton_PS_1088"))
        self.label_4 = QtGui.QLabel(self.groupBox_2)
        self.label_4.setGeometry(QtCore.QRect(10, 0, 201, 21))
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Preferred, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_4.sizePolicy().hasHeightForWidth())
        self.label_4.setSizePolicy(sizePolicy)
        self.label_4.setText(QtGui.QApplication.translate("Main", "Packet size", None, QtGui.QApplication.UnicodeUTF8))
        self.label_4.setScaledContents(False)
        self.label_4.setWordWrap(True)
        self.label_4.setObjectName(_fromUtf8("label_4"))
        self.radioButton_PS_1500 = QtGui.QRadioButton(self.groupBox_2)
        self.radioButton_PS_1500.setGeometry(QtCore.QRect(20, 120, 114, 22))
        self.radioButton_PS_1500.setText(QtGui.QApplication.translate("Main", "1500 bytes", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_PS_1500.setObjectName(_fromUtf8("radioButton_PS_1500"))
        self.radioButton_PS_192 = QtGui.QRadioButton(self.groupBox_2)
        self.radioButton_PS_192.setGeometry(QtCore.QRect(20, 40, 114, 22))
        self.radioButton_PS_192.setText(QtGui.QApplication.translate("Main", "192 bytes", None, QtGui.QApplication.UnicodeUTF8))
        self.radioButton_PS_192.setChecked(False)
        self.radioButton_PS_192.setObjectName(_fromUtf8("radioButton_PS_192"))
        self.label_5 = QtGui.QLabel(Main)
        self.label_5.setGeometry(QtCore.QRect(10, 390, 261, 17))
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_5.sizePolicy().hasHeightForWidth())
        self.label_5.setSizePolicy(sizePolicy)
        self.label_5.setText(QtGui.QApplication.translate("Main", "Real-time packet forwarding speed", None, QtGui.QApplication.UnicodeUTF8))
        self.label_5.setScaledContents(False)
        self.label_5.setAlignment(QtCore.Qt.AlignCenter)
        self.label_5.setObjectName(_fromUtf8("label_5"))
        self.pushButton_Router = QtGui.QPushButton(Main)
        self.pushButton_Router.setGeometry(QtCore.QRect(10, 650, 111, 27))
        self.pushButton_Router.setText(QtGui.QApplication.translate("Main", "Reset Router", None, QtGui.QApplication.UnicodeUTF8))
        self.pushButton_Router.setAutoDefault(True)
        self.pushButton_Router.setObjectName(_fromUtf8("pushButton_Router"))
        self.pushButton_TGen = QtGui.QPushButton(Main)
        self.pushButton_TGen.setGeometry(QtCore.QRect(300, 650, 181, 27))
        self.pushButton_TGen.setText(QtGui.QApplication.translate("Main", "Reset Traffic Generator", None, QtGui.QApplication.UnicodeUTF8))
        self.pushButton_TGen.setObjectName(_fromUtf8("pushButton_TGen"))
        self.pushButton_Monitor = QtGui.QPushButton(Main)
        self.pushButton_Monitor.setGeometry(QtCore.QRect(130, 650, 161, 27))
        self.pushButton_Monitor.setText(QtGui.QApplication.translate("Main", "Reset Router Monitor", None, QtGui.QApplication.UnicodeUTF8))
        self.pushButton_Monitor.setAutoDefault(True)
        self.pushButton_Monitor.setObjectName(_fromUtf8("pushButton_Monitor"))
        self.frame_plot = QtGui.QFrame(Main)
        self.frame_plot.setGeometry(QtCore.QRect(10, 410, 711, 231))
        self.frame_plot.setFrameShape(QtGui.QFrame.StyledPanel)
        self.frame_plot.setFrameShadow(QtGui.QFrame.Raised)
        self.frame_plot.setObjectName(_fromUtf8("frame_plot"))
        self.label_Header = QtGui.QLabel(Main)
        self.label_Header.setGeometry(QtCore.QRect(230, 30, 281, 301))
        self.label_Header.setText(QtGui.QApplication.translate("Main", "Header", None, QtGui.QApplication.UnicodeUTF8))
        self.label_Header.setObjectName(_fromUtf8("label_Header"))
        self.label_DAG = QtGui.QLabel(Main)
        self.label_DAG.setGeometry(QtCore.QRect(30, 110, 151, 141))
        self.label_DAG.setText(QtGui.QApplication.translate("Main", "DAG", None, QtGui.QApplication.UnicodeUTF8))
        self.label_DAG.setObjectName(_fromUtf8("label_DAG"))
        self.checkBox_IP = QtGui.QCheckBox(Main)
        self.checkBox_IP.setGeometry(QtCore.QRect(520, 390, 201, 21))
        self.checkBox_IP.setText(QtGui.QApplication.translate("Main", "Show IP forwarding speed", None, QtGui.QApplication.UnicodeUTF8))
        self.checkBox_IP.setObjectName(_fromUtf8("checkBox_IP"))

        self.retranslateUi(Main)
        QtCore.QMetaObject.connectSlotsByName(Main)

    def retranslateUi(self, Main):
        pass

