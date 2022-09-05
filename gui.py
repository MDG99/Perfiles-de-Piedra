import time
import serial
import tkinter.font as TkFont
import serial.tools.list_ports

from tkinter import *
from tkinter import ttk
from tkinter import messagebox




class gui():

    def __init__(self):

        self.ventana = Tk()
        #self.ventana.geometry("300x200")
        self.ventana.resizable(width=0, height=0) #Ventana no redimensionable
        self.ventana.title("Control del motor DC")

        #Listamos los dispositivos conectados al puerto COM
        self.port_names = [comport.device for comport in serial.tools.list_ports.comports()]
        self.micro = None

        #componentes del GUI
        self.fontformat_title =TkFont.Font(family="Arial", size=15, weight="bold")
        self.fontformat_sub = TkFont.Font(family="Arial", size=12)

        #Botones
        self.btnConectar = Button(self.ventana, text="Conectar", width=15, command=self.connect)
        self.btnConectar.grid(column=2, row=1, padx=5, pady=5)

        self.btnDesconectar = Button(self.ventana, text="Desconectar", width=15, command=self.disconnect)
        self.btnDesconectar.grid(column=2, row=2, padx=5, pady=5)

        self.btnFrente = Button(self.ventana, text="Frente", width=15, command=self.forward)
        self.btnFrente.grid(column=0, row=4, padx=5, pady=5)

        self.btnReversa = Button(self.ventana, text="Reversa", width=15, command=self.backward)
        self.btnReversa.grid(column=1, row=4, padx=5, pady=5)

        self.btnParo = Button(self.ventana, text="Paro", width=15, command=self.stop)
        self.btnParo.grid(column=2, row=4, padx=5, pady=5)

        self.btnVelMas = Button(self.ventana, text="+", width=5, command=self.add)
        self.btnVelMas.grid(column=0, row=5, padx=5, pady=5)

        self.btnVelMenos = Button(self.ventana, text="-", width=5, command=self.less)
        self.btnVelMenos.grid(column=2, row=5, padx=5, pady=5)

        self.btnPos = Button(self.ventana, text="Enviar", width=10, command=self.sendData)
        self.btnPos.grid(column=2, row=6, padx=5, pady=5)

        self.btnshow = Button(self.ventana, text="Mostrar", width=10, command=self.show)
        self.btnshow.grid(column=2, row=11, padx=5, pady=5)

        #Texto
        self.lbltitle0 = Label(self.ventana, text="Conectividad", font=self.fontformat_title)
        self.lbltitle0.grid(column=0, row=0, padx=5, pady=5, columnspan=3)

        self.lblsubtitle0 = Label(self.ventana, text="Dispositivos", font=self.fontformat_sub)
        self.lblsubtitle0.grid(column=0, row=1, padx=5, pady=5)

        self.lbltitle1 = Label(self.ventana, text="Funciones Básicas", font=self.fontformat_title)
        self.lbltitle1.grid(column=0, row=3, padx=5, pady=5, columnspan=3)

        self.lblvel = Label(self.ventana, text="Vel.", font=self.fontformat_sub)
        self.lblvel.grid(column=1, row=5, padx=5, pady=5)

        self.lblpos = Label(self.ventana, text="Mover (Grados)", font=self.fontformat_sub)
        self.lblpos.grid(column=0, row=6, padx=5, pady=5)

        self.lbltitle2 = Label(self.ventana, text="Funciones de Monitoreo", font=self.fontformat_title)
        self.lbltitle2.grid(column=0, row=8, padx=5, pady=15, columnspan=3)

        self.lblsubtitle1 = Label(self.ventana, text="Posición Actual del Encoder (Grados)", font=self.fontformat_sub)
        self.lblsubtitle1.grid(column=0, row=9, padx=5, pady=5, columnspan=2, sticky="e")

        self.lblcurrent_pos = Label(self.ventana, text=f' ', font=self.fontformat_sub)
        self.lblcurrent_pos.grid(column=2, row=9, padx=5, pady=5)

        self.lblsubtitle2 = Label(self.ventana, text="Velocidad Actual (PWM)", font=self.fontformat_sub)
        self.lblsubtitle2.grid(column=0, row=10, padx=5, pady=5, columnspan=2, sticky="e")

        self.lblcurrent_vel = Label(self.ventana, text=f' ', font=self.fontformat_sub)
        self.lblcurrent_vel.grid(column=2, row=10, padx=5, pady=5)

        #Textboxes
        self.txt_angle = Entry(self.ventana, width=10)
        self.txt_angle.grid(column=1, row=6)

        #Combobox
        self.cmbPorts = ttk.Combobox(self.ventana, width=10, values=self.port_names)
        self.cmbPorts.grid(column=1, row=1)

    def connect(self):
        serial_port = self.cmbPorts.get()
        baud_rate = 9600

        # Conexión inicial
        try:
            self.micro = serial.Serial(serial_port, baud_rate)
            messagebox.showinfo(message="Conexión iniciada", title="Puerto Serial")
            print('Conexión inicial')
            time.sleep(2)
        except:
            messagebox.showerror(message="No se pudo conectar", title="Puerto Serial")
            pass

    def disconnect(self):
        try:
            self.micro.close()
            messagebox.showwarning(message="Dispositivo desconectado", title="Puerto Serial")
        except:
            pass

    def forward(self):
        #Enviamos la instrucción
        self.micro.write(b"F\n")

    def backward(self):
        # Enviamos la instrucción
        self.micro.write(b"R\n")

    def stop(self):
        # Enviamos la instrucción
        self.micro.write(b"S\n")

    def add(self):
        # Enviamos la instrucción
        self.micro.write(b"IS\n")

    def less(self):
        # Enviamos la instrucción
        self.micro.write(b"DS\n")

    def sendData(self):
        txt_degree = self.txt_angle.get()

        try:
            degree = str(txt_degree)
            degree = bytes(degree, 'UTF-8')
        except:
            degree = b'0'

        self.txt_angle.delete(0, 'end') #Eliminamos el texto del comboBox

        #Mandamos la información
        instruccion = b"MT " + degree + b"\n"

        self.micro.write(instruccion)

        #print(instruccion.decode('utf-8'))#Comprobamos que se envió en el formato adecuado

    def show(self):
        #Aquí se lee el micro

        # El puerto serie nos da un dato del tipo byte, con el terminado de línea \r\n
        # Por lo tanto, es necesario decodificarlo para convertirlo a String y eliminar
        # Los terminadores de línea

        self.micro.write(b"GP\n") #Instrucción de lectura de posición
        self.micro.flushInput()
        current_pos = self.micro.readline() #Leemos la posición
        current_pos = current_pos.decode('utf-8')
        current_pos = current_pos.rstrip("\r\n")
        current_pos = current_pos

        self.micro.write(b"GS\n")  # Instrucción de lectura de velocidad
        self.micro.flushInput()
        current_vel = self.micro.readline()  # Leemos la velocidad
        current_vel = current_vel.decode('utf-8')
        current_vel = current_vel.rstrip("\r\n")
        current_vel = int(current_vel)

        self.lblcurrent_pos.config(text=f' {current_pos}° ')
        self.lblcurrent_vel.config(text=f' {current_vel} ')


my_app = gui()
my_app.ventana.mainloop()


#Funcion desactualizada para la lectura
#def puerto_serial():
#    try:
#        global flag
#        micro.flushInput()
#        orden = micro.readline()
#        print(orden)
#        if orden == b"A":
#            flag = 1
#            print(orden)
#    except:
#        time.sleep(1)
#        print("Hola")
#        pass

