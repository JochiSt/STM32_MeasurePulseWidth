# -*- coding: utf-8 -*-

# -*- coding: utf-8 -*-

import numpy as np
import matplotlib.pyplot as plt

#plt.rcParams['text.usetex'] = True

import scipy.stats as stats

def VCD_pulseWidth(vcd_filename):
    duration = {}
    names = {}
    F_sample = 12E6         # 12 MHz
    with open(vcd_filename) as f:
        lines = f.readlines()
        old_level = {}
        old_timestamp = {}
        timescale = 0

        for line in lines:
            line = line[:-1]
            if "$timescale" in line:
                line = line.replace('$timescale', '').replace('$end', '')
                line = line.replace('ps', 'e-12')
                line = line.replace('ns', 'e-9')
                line = line.replace('ms', 'e-3')
                line = line.replace(' ', '')
                timescale = float(line)
            elif "Acquisition" in line:
                #Acquisition with 1/8 channels at 12 MHz
                F_sample =  float(line.split(" ")[7])
                if "MHz" in line:
                    F_sample *= 1e6
                elif "kHz" in line:
                    F_sample *= 1e3
                else:
                    F_sample *= 1e6
            elif "$var wire" in line:
                symbol = line.split(" ")[3]
                name = " ".join(line.split(" ")[4:])
                name = name.replace(' $end', '')
                duration[symbol] = []
                names[symbol] = name

                old_level[symbol] = -1
                old_timestamp[symbol] = 0

                print("Found symbol %s -> %s"%(symbol, name))

            elif line[0] == '#':
                line = line[1:]
                symbol = line[-1]
                line = line.replace(symbol,'')
                try:
                    timestamp, level = line.split(' ')
                    timestamp = int(timestamp)
                    level = int(level)
                except:
                    continue

                if level != old_level:
                    if level == 0 and old_level[symbol] == 1:   # we are interested in the duration of the high
                                                                # signal
                        deltaT = (timestamp - old_timestamp[symbol])*timescale*1e6 # in us
                        duration[symbol].append( deltaT )

                    old_timestamp[symbol] = timestamp
                    old_level[symbol] = level

    return names, duration, F_sample

def analyse_pulse_width(vcd_files):

    for vcd_filename in vcd_files:
        names, duration, F_sample = VCD_pulseWidth(vcd_filename)
        break   # just analyse a single file at the moment

    fig, ax = plt.subplots(1, 1, figsize=(8, 5))

    for key in names.keys():
        # exclude trigger, because the time duration might be not that
        # meaningful

        print("Processing", names[key], key)

        if names[key] == "Trigger":
            continue

        # get the single array from the dictionary
        a_duration = np.array(duration[key])

        if len(a_duration) == 0:
            print("\tNo pulse found")
            continue

        mean_duration = np.mean(a_duration)
        std_duration = np.std(a_duration)

        # define the histogram range (exclude the outliers)
        hist_range = [
                # minimal side (duration cannot be lower than 0)
                max(mean_duration-5*std_duration, 0),
                # maximal side
                mean_duration+5*std_duration
                ]
        # get the histogram range as a number
        delta_hist_range = hist_range[1]-hist_range[0]
        # calculate the number of bins needed to cover the range
        hist_bins = int( np.floor(delta_hist_range*1e-6 /(1./(F_sample/4.))))
        # calculate the bin width
        bin_width = delta_hist_range / hist_bins * 1000

        mean_freq = 1/(mean_duration*1e-6)
        std_freq  = 1/(mean_duration*1e-6)**2 * std_duration*1e-6


        # histogram the data
        biny, binx, _ = plt.hist(a_duration, bins=hist_bins,
                                 range=hist_range,
                                 label="$\mathbf{%s}$"%(names[key])
                                         +"$\quad N = %d$"%(len(a_duration))
                                         +"\n"
                                         +"$\Delta T =%6.2f \pm %6.2f \, \mu \mathrm{s}$"%(mean_duration, std_duration)
                                         +"\n"
                                         +"$f =%6.2f \pm %6.2f \, \mathrm{kHz}$"%( mean_freq/1000, std_freq/1000)
                                         +"\n"
                                         +"$\Delta T_\mathrm{bin} =%4.2f \, \mathrm{ns}$"%(bin_width)
                                         )

        #x = np.linspace(binx.min(), binx.max(), 100)
        #plt.plot(x, stats.norm.pdf(x, mean_duration, std_duration) * np.max(biny) )

        if names[key] == "Clock":
            plt.axvline(mean_duration  , color="black", linestyle='--', linewidth=0.7)
            plt.axvline(2*mean_duration, color="black", linestyle='--', linewidth=0.7)


    ax.set_yscale('log') #, nonposy='clip')

    plt.title("pulse width distribution")
    plt.ylabel("entries")
    plt.xlabel(r'pulse width / \textmu s')
    plt.legend()
    plt.savefig( vcd_files[0].replace('.vcd', '.png') )
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":

    analyse_pulse_width([
                "manchester_out_20230612_1316.vcd",
             ])
