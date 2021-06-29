# -*- coding: utf-8 -*-
"""
Created on Thu Jan 02 13:47:01 2020

@authors:   
            Carlos Silva 1160628
            Diogo Pinto  1160736
"""
# Libraries
from PyQt4 import QtGui, uic
from PyQt4.QtGui import QFileDialog, QTextEdit, QPixmap
from PyQt4.Qwt5.Qwt import QwtPlot, QwtPlotCurve
from PyQt4 import QtCore as QtCore
import numpy as np
import time
import urllib2
import csv
import turtle
import math
import pygame

def _translate(context, text, disambig):
    return QtGui.QApplication.translate(context, text, disambig, _encoding)
def _translate(context, text, disambig):
    return QtGui.QApplication.translate(context, text, disambig)
    self.retranslateUi(MainWindow)
    
class MyWindow(QtGui.QMainWindow):
    
    def __init__(self):
        #Initiate App
        super(MyWindow, self).__init__()
        uic.loadUi('App_Pikasso_Project.ui', self)
        self.show()
        
        #Global variables
        global penFlag
        penFlag = 0
        global joyFlag
        joyFlag = 0
        global mouseFlag
        mouseFlag = 0
        
        # Arrow buttons assign to functions
        self.up_Button.clicked.connect(self.up_arrow)
        self.upRight_Button.clicked.connect(self.upRight_arrow)
        self.upLeft_Button.clicked.connect(self.upLeft_arrow)
        self.left_Button.clicked.connect(self.left_arrow)
        self.right_Button.clicked.connect(self.right_arrow)
        self.down_Button.clicked.connect(self.down_arrow)
        self.downLeft_Button.clicked.connect(self.downLeft_arrow)
        self.downRight_Button.clicked.connect(self.downRight_arrow)
        self.penMove_Button.clicked.connect(self.penMove_arrow)
        self.readCSV_Button.clicked.connect(self.read_csv)
        self.goto_Button.clicked.connect(self.goto)
        self.poly_Button.clicked.connect(self.create_polygon)
        self.square_Button.clicked.connect(self.create_square)
        self.triangle_Button.clicked.connect(self.create_triangle)
        self.star_Button.clicked.connect(self.create_star)
        self.circle_Button.clicked.connect(self.create_circle)
        self.joy_Button.clicked.connect(self.joy_handle)
        self.mouse_Button.clicked.connect(self.mouse_handle)
        
    def up_arrow(self):
        urllib2.urlopen('http://192.168.4.1/motors?dir_x=r&dist_x=200&speed_x=50&dir_y=u&dist_y=200&speed_y=50')
        
    def upRight_arrow(self):
        urllib2.urlopen('http://192.168.4.1/motors?dir_x=r&dist_x=200&speed_x=50&dir_y=u&dist_y=0&speed_y=50')
        
    def upLeft_arrow(self):
        urllib2.urlopen('http://192.168.4.1/motors?dir_x=r&dist_x=0&speed_x=50&dir_y=u&dist_y=200&speed_y=50')
        
    def left_arrow(self):
        urllib2.urlopen('http://192.168.4.1/motors?dir_x=l&dist_x=200&speed_x=50&dir_y=u&dist_y=200&speed_y=50')
        
    def right_arrow(self):
        urllib2.urlopen('http://192.168.4.1/motors?dir_x=r&dist_x=200&speed_x=50&dir_y=d&dist_y=200&speed_y=50')
        
    def down_arrow(self):
        urllib2.urlopen('http://192.168.4.1/motors?dir_x=l&dist_x=200&speed_x=50&dir_y=d&dist_y=200&speed_y=50')
        
    def downLeft_arrow(self):
        urllib2.urlopen('http://192.168.4.1/motors?dir_x=l&dist_x=200&speed_x=50&dir_y=u&dist_y=0&speed_y=50')
        
    def downRight_arrow(self):
        urllib2.urlopen('http://192.168.4.1/motors?dir_x=r&dist_x=0&speed_x=50&dir_y=d&dist_y=200&speed_y=50')
        
    def penMove_arrow(self):
        global penFlag
        if(penFlag == 0):
            urllib2.urlopen('http://192.168.4.1/servo?pos=pen_down')
            penFlag = 1
        else:
            urllib2.urlopen('http://192.168.4.1/servo?pos=pen_up')
            penFlag = 0
            
    def draw_array(self, points):
    # Rotacao dos eixos, pontos em formato cartesiano alterado
        for i in range(0, len(points)):
            point_x = points[i][0]
            point_y = points[i][1]
            
            points[i][0] = (point_x * math.cos(math.pi/4) + point_y * math.sin(math.pi/4))/0.014
            points[i][1] = (point_x * -1 * math.sin(pi/4) + point_y * math.cos(pi/4))/0.014       
        
        print points 
        
        for i in range(0, len(points) - 1):
            dist_x = float(points[i+1][0] - points[i][0])
            dist_y = float(points[i+1][1] - points[i][1])
            print(i)
            print('\n\n')        
            
            print dist_x
            print dist_y
            
            # Primeiro e Quarto Quadrante (arco-tangente)
            if(dist_x > 0):
                beta = math.atan(dist_y/dist_x)
            # Cima
            elif((dist_x == 0) and (dist_y > 0)):
                beta = pi/2
            # Baixo
            elif((dist_x == 0) and (dist_y < 0)):
                beta = -pi/2
            # Esquerda
            elif((dist_y == 0) and (dist_x > 0)):
                beta = 0
            # Direita
            elif((dist_y == 0) and (dist_x < 0)):
                beta = pi
            # Segundo e Terceiro Quadrante (arco-tangente + pi)
            else:
                beta = math.atan(dist_y/dist_x) + pi
            
            print math.degrees(beta)
             
            # Primeiro Quadrante
            if(((beta >= 0) and (beta < pi/2)) or ((beta < -3*pi/2) and (beta >= -2*pi))):
                dir_x = 'r'
                dir_y = 'u'
            # Segundo Quadrante
            elif((beta >= pi/2) and (beta < pi) or ((beta < -pi) and (beta >= -3*pi/2))):
                dir_x = 'l'
                dir_y = 'u'
            #Terceiro Quadrante
            elif((beta >= pi) and (beta < 3*pi/2) or ((beta < -pi/2) and (beta >= -pi))):
                dir_x = 'l'
                dir_y = 'd'
            #Quarto Quadrante
            elif((beta >= 3*pi/2) and (beta < 2*pi) or ((beta < 0) and (beta >= -pi/2))):
                dir_x = 'r'
                dir_y = 'd'
            
            speed_x = int(60 / abs(math.cos(beta)))
            if(speed_x > 255): speed_x = 255
            speed_y = int(60 / abs(math.sin(beta)))
            if(speed_y > 255): speed_y = 255
            
            dist_x = int(abs(dist_x))
            dist_y = int(abs(dist_y))
            
            command = 'http://192.168.4.1/motors?dir_x=' + dir_x + '&dist_x=' + str(dist_x) + '&speed_x=' + str(speed_x) + '&dir_y=' + dir_y + '&dist_y=' + str(dist_y) + '&speed_y=' + str(speed_y)
            
            urllib2.urlopen(command)
            
            print command
            
            time.sleep(0.2)
        
    def draw_polygon(self, lados, radius, angle):
        angledif = 2*pi / lados
        points = np.zeros((lados+1, 2))
        points[0] = ([0, radius])
        
        for i in range(0, lados+1):
            angle = angle - angledif
            points[i][0] = radius * math.cos(angle)
            points[i][1] = radius * math.sin(angle)  
            
        print points
            
        self.draw_array(points)
        
    def read_csv(self):
        filename = QFileDialog.getOpenFileName(self,
                                               'Open File', 
                                               '~/Desktop',
                                               'Data files (*.txt)')
                                               
        with open(filename) as csv_file:
            csv_reader = csv.reader(csv_file, delimiter=',')
            num_lines = (sum(1 for line in csv_reader))
            points = np.zeros((num_lines-1, 2))
            csv_file.seek(0)
            csv_reader = csv.reader(csv_file, delimiter=',')
            i=0
            for row in csv_reader:
                if(i != 0):
                    points[i-1][0] = float(row[0])
                    points[i-1][1] = float(row[1])
                i+=1
            print points
        self.draw_array(points)
        
    def goto(self):
        x = self.gotoX_Value.value()
        y = self.gotoY_Value.value()
        goto_array = [[0, 0], [x, y]]
        self.draw_array(goto_array)
        
    def create_polygon(self):
        lados = self.polyLados_Value.value()
        raio = self.polyRaio_Value.value()
        angle = self.polyAngle_Value.value()*pi/180
        self.draw_polygon(lados, raio, angle)
        
    def create_square(self):
        lados = 4
        raio = self.squareLado_Value.value()/math.sqrt(2)
        angle = self.squareAngle_Value.value()*pi/180
        self.draw_polygon(lados, raio, angle)        
        
    def create_triangle(self):
        lados = 3
        raio = self.triangleLado_Value.value()/math.sqrt(3)
        angle = self.triangleAngle_Value.value()*pi/180
        self.draw_polygon(lados, raio, angle)
        
    def create_star(self):
        raios = self.starRaios_Value.value()
        angle = self.starAngle_Value.value()*pi/180
        radius = self.startRaio_Value.value()
        
        angledif = 2*pi / raios
        points = np.zeros((raios*2+1, 2))
        
        points[0] = ([0, 0])
        
        for i in range(1, raios*2+1):
            if((i % 2) == 0):
                points[i][0] = 0
                points[i][1] = 0
            else:
                points[i][0] = radius * math.cos(angle)
                points[i][1] = radius * math.sin(angle)
                angle = angle - angledif
            
        print points
        
        self.draw_array(points)
        
    def create_circle(self):
        radius = self.circleRaio_Value.value()
        angleI = self.circleAngleI_Value.value()*pi/180
        angleF = self.circleAngleF_Value.value()*pi/180

        lados = int((angleF - angleI)*26 / (2*pi))
        angledif = (angleF - angleI) / lados
        points = np.zeros((lados+1, 2))
        
        for i in range(0, lados+1):
            points[i][0] = radius * math.cos(angleI)
            points[i][1] = radius * math.sin(angleI)
            angleI = angleI + angledif
            
        print points
        
        self.draw_array(points)
        
    def joy_handle(self):
        global joyFlag
        global penFlag
        
        if(joyFlag == 0):
            joyFlag = 1
            self.on_off_Joy.setPixmap(QtGui.QPixmap("C:/Users/litos/Desktop/ISEP/LABSI/Projeto/Icons/on.png"))
            
            pygame.init()

            white = (255, 255, 255)
            red = (255, 0, 0)
            
            gameDisplay = pygame.display.set_mode((800, 600))
            pygame.display.update()
            
            gameExit = False
            
            ## Commands
            joy_x = 0
            joy_square = 3
            joy_triangle = 0
            joy_circle = 1
            joy_r1 = 5
            joy_l1 = 4
            joy_select = 6
            joy_start = 7
            joy_lAnalog = 8
            joy_rAnalog = 9
            
            joysticks = []

            clock = pygame.time.Clock()
            
            for i in range(0, pygame.joystick.get_count()):
                joysticks.append(pygame.joystick.Joystick(i))
                joysticks[-1].init()
                print("Detected Joystick ", joysticks[-1].get_name())
                
            my_joystick = pygame.joystick.Joystick(0)
            my_joystick.init()
            
            while not gameExit:
                clock.tick(60)
                
                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        gameExit = True
                        
                if event.type == pygame.JOYBUTTONDOWN:
                    print event.button
                    if event.button == joy_triangle:
                        urllib2.urlopen('http://192.168.4.1/servo?pos=pen_up')
                    if event.button == joy_square:
                        urllib2.urlopen('http://192.168.4.1/servo?pos=pen_down')
                    if event.button == joy_circle:
                        joyFlag = 0
                        self.on_off_Joy.setPixmap(QtGui.QPixmap("C:/Users/litos/Desktop/ISEP/LABSI/Projeto/Icons/off.png"))
                        pygame.quit()
                        
                if event.type == pygame.JOYAXISMOTION:
                    x = my_joystick.get_axis(0) #Left X Axis (-1 left, 1 right)
                    y = -1*my_joystick.get_axis(1) #Left Y Axis (-1 Up, 1 Down)
                    goto_array = [[0, 0], [x, y]]
                    self.draw_array(goto_array)
                
                gameDisplay.fill(red)
                pygame.display.update()
            
            pygame.quit()
            quit()
        
    def mouse_handle(self):
        global mouseFlag
        
        if(mouseFlag == 0):
            mouseFlag = 1
            self.on_off_Mouse.setPixmap(QtGui.QPixmap("C:/Users/litos/Desktop/ISEP/LABSI/Projeto/Icons/on.png"))
            
        else:
            mouseFlag = 0
            self.on_off_Mouse.setPixmap(QtGui.QPixmap("C:/Users/litos/Desktop/ISEP/LABSI/Projeto/Icons/off.png"))
        
if __name__ == '__main__':
    app = QtGui.QApplication(sys.argv)
    window = MyWindow()
    sys.exit(app.exec_())