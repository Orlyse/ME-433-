import csv
import pandas
import numpy as np
import matplotlib.pyplot as plt

def open_file(filename):
    with open(filename, 'r') as file:
        data = csv.reader(file)
        x = []
        y = []
        for row in data:
            x.append(float(row[0]))
            y.append(float(row[1]))

    return np.array(x), np.array(y)

def calculate_sr(signal_time):
    sr =  signal_time[-1] - signal_time[0]
    return sr/len(signal_time)

def compute_fft(signal, sr):
    y = signal
    n = len(y)
    #   k = np.arange(n)
    #   T = n/sr
    #   frq = k/T
    #   frq = frq[range(int(n/2))]
    #   Y = np.fft.fft(y)/n
    #   Y = Y[range(int(n/2))]
    frq = np.fft.fftfreq(n, d=1/sr)
    Y = np.fft.fft(signal)/n
    
    return frq, Y

def plot_data(Ax, Ay ,Bx, By, Cx, Cy, Dx, Dy, c, t):
    plt.subplot(2, 2, 1)
    plt.plot(Ax, Ay, color=c)
    plt.xlabel(r'Time')
    plt.ylabel(r'Signal')
    plt.title(r'Signal A')


    plt.subplot(2, 2, 2)
    plt.plot(Bx, By, color=c)
    plt.xlabel(r'Time')
    plt.ylabel(r'Signal')
    plt.title(r'Signal B')


    plt.subplot(2, 2, 3)
    plt.plot(Cx, Cy, color=c)
    plt.xlabel(r'Time')
    plt.ylabel(r'Signal')
    plt.title(r'Signal C')


    plt.subplot(2, 2, 4)
    plt.plot(Dx, Dy, color=c)
    plt.xlabel(r'Time')
    plt.ylabel(r'Signal')
    plt.title(r'Signal D')

    plt.suptitle(t, fontsize=15)
    plt.tight_layout(pad=0.25)
    plt.show()


def plot_fft(fa, ya, fb, yb, fc, yc, fd, yd):
    plt.subplot(2, 2, 1)
    plt.semilogy(fa, abs(ya))
    plt.xlabel(r'Frequency in Hz')
    plt.ylabel('|Y(freq)|')
    plt.xticks(rotation=90)
    plt.title(r'Signal A')


    plt.subplot(2, 2, 2)
    plt.semilogy(fb, abs(yb))
    plt.xlabel(r'Frequency in Hz')
    plt.ylabel('|Y(freq)|')
    plt.xticks(rotation=90)
    plt.title(r'Signal B')


    plt.subplot(2, 2, 3)
    plt.semilogy(fc, abs(yc))
    plt.xlabel(r'Frequency in Hz')
    plt.ylabel('|Y(freq)|')
    plt.xticks(rotation=90)
    plt.title(r'Signal C')


    plt.subplot(2, 2, 4)
    plt.semilogy(fd, abs(yd))
    plt.xlabel(r'Frequency in Hz')
    plt.ylabel('|Y(freq)|')
    plt.xticks(rotation=90)
    plt.title(r'Signal D')

    plt.suptitle('FFT PLOTS', fontsize=15)
    plt.tight_layout(pad=0.25)
    plt.show()    

def moving_avg(data_x, data_y, N):

    new_data_x = []
    new_data_y = []

    for i in range(len(data_x)-N):
        total_x = 0
        total_y = 0

        for j in range(N):
            total_x = total_x + data_x[i+j]
            total_y = total_y + data_y[i+j]
        
        avg_x = total_x/N
        new_data_x.append(avg_x)

        avg_y = total_y/N
        new_data_y.append(avg_y)

    return np.array(new_data_x), np.array(new_data_y)

def low_pass_iir(data_x, data_y, A, B):
    nx = []
    ny = []

    for i in range(len(data_x)-1):
        tx = A*data_x[i] + B*data_x[i+1]
        nx.append(tx)

        ty = A*data_y[i] + B*data_y[i+1]
        ny.append(ty)

    
    return np.array(nx), np.array(ny)

def low_pass_sinc(signal, coeffs):   
    N_fil = len(coeffs)
    filt = []

    for i in range(len(signal)):
        tot = 0
        for j in range(N_fil):
            if i-j >=0:
                tot +=  coeffs[j]*signal[i-j]
        filt.append(tot)

    return np.array(filt)
    
sigA_x, sigA_y = open_file("sigA.csv")
sr_A = calculate_sr(sigA_x)
frqA, YA = compute_fft(sigA_y, sr_A)
#   print(frqA)

sigB_x, sigB_y = open_file("sigB.csv")
sr_B = calculate_sr(sigB_x)
frqB, YB = compute_fft(sigB_y, sr_B)

sigC_x, sigC_y = open_file("sigC.csv")
sr_C = calculate_sr(sigC_x)
frqC, YC = compute_fft(sigC_y, sr_C)

sigD_x, sigD_y = open_file("sigD.csv")
sr_D = calculate_sr(sigD_x)
frqD, YD = compute_fft(sigD_y, sr_D)

print(f'sampling rates: A = {sr_A} B = {sr_B} C = {sr_C} D = {sr_D}')
#   plot_data(sigA_x, sigA_y, sigB_x, sigB_y, sigC_x, sigC_y, sigD_x, sigD_y, 'black', 'ORIGINAL UNFILTERED DATA')
#   plot_fft(frqA, YA, frqB, YB, frqC, YC, frqD, YD)

avg_ax, avg_ay = moving_avg(sigA_x, sigA_y, 75)
#   print(len(avg_ax))
avg_bx, avg_by = moving_avg(sigB_x, sigB_y, 75)
avg_cx, avg_cy = moving_avg(sigC_x, sigC_y, 75)
avg_dx, avg_dy = moving_avg(sigD_x, sigD_y, 75)

#   plot_data(avg_ax, avg_ay, avg_bx, avg_by, avg_cx, avg_cy, avg_dx, avg_dy, 'green', 'Moving average N=75')


iir_ax, iir_ay = low_pass_iir(sigA_x, sigA_y, 0.355, 0.645)
iir_bx, iir_by = low_pass_iir(sigB_x, sigB_y, 0.3, 0.7)
iir_cx, iir_cy = low_pass_iir(sigC_x, sigC_y, 0.3, 0.7)
iir_dx, iir_dy = low_pass_iir(sigD_x, sigD_y, 0.3, 0.7)

#   plot_data(iir_ax, iir_ay, iir_bx, iir_by, iir_cx, iir_cy, iir_dx, iir_dy, 'grey', 'IIR A=0.355, B=0.645')

h = [-0.000000000000000001,
        0.000228536047965746,
        0.000969810388449305,
        0.002355955521686686,
        0.004565850432870298,
        0.007789821209440546,
        0.012184295699380641,
        0.017824312888543335,
        0.024662706168791657,
        0.032504153691186657,
        0.041000178705238192,
        0.049667913970645065,
        0.057931555698216348,
        0.065181583878765062,
        0.070843683462622126,
        0.074447419646362908,
        0.075684445179670795,
        0.074447419646362908,
        0.070843683462622126,
        0.065181583878765076,
        0.057931555698216362,
        0.049667913970645065,
        0.041000178705238220,
        0.032504153691186678,
        0.024662706168791671,
        0.017824312888543335,
        0.012184295699380638,
        0.007789821209440557,
        0.004565850432870304,
        0.002355955521686686,
        0.000969810388449307,
        0.000228536047965748,
        -0.000000000000000001
        ] # cutoff = 0.01 hz, transition = 0.14, window=blackman

h1 = [
    0.000000000000000000,
    0.001327222896911332,
    0.004650252250657945,
    0.011749802608142372,
    0.023779221956107037,
    0.040723451189874783,
    0.061156926116234515,
    0.082403674188359019,
    0.101082211156672028,
    0.113897378800497828,
    0.118459717673086232,
    0.113897378800497842,
    0.101082211156672055,
    0.082403674188359033,
    0.061156926116234543,
    0.040723451189874790,
    0.023779221956107044,
    0.011749802608142373,
    0.004650252250657947,
    0.001327222896911332,
    0.000000000000000000,
]  # cutoff = 0.05, transition = 0.15, window=hamming

a = low_pass_sinc(sigA_y, h)
b = low_pass_sinc(sigB_y, h)
c = low_pass_sinc(sigC_y, h)
d = low_pass_sinc(sigD_y, h)

plot_data(sigA_x, a, sigB_x, b, sigC_x, c, sigD_x, d, 'purple', 'LOW PASS FILTER(SINC), CUTOFF FREQ=0.01HZ, TRANSITION BW = 0.14, WINDOW_TYPE=BLACKMAN')