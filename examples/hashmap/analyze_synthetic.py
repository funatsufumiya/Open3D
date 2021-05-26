import os
import numpy as np
import argparse

import matplotlib.pyplot as plt
plt.rcParams.update({
    "text.usetex": True,
    "font.family": "sans-serif",
    "font.sans-serif": ["Helvetica"]})
# for Palatino and other serif fonts use:
plt.rcParams.update({
    "text.usetex": True,
    "font.family": "serif",
    "font.serif": ["Palatino"],
})

iters = [1000, 10000, 100000, 1000000]
map_iters_row = {1000: 0, 10000: 1, 100000: 2, 1000000: 3}

channels = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096]
map_channels_col = lambda x: int(np.log2(x))
density = [0.1, 0.99]

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('path')
    args = parser.parse_args()

    files = os.listdir(args.path)

    # a stats dict is organized as follows:
    # density
    # |_______ find (n, c) array
    # |_______ insert (n, c) array
    # |_______ [optional] activate (n, c) array
    stats_ours = {}
    stats_stdgpu = {}
    for f in sorted(files):
        with open(os.path.join(args.path, f)) as f:
            content = f.readlines()

        local_dict = {}
        for line in content:
            elems = line.strip().split(' ')
            key = elems[2]
            val = float(elems[3])
            local_dict[key] = val

        density = local_dict['density']
        n = local_dict['n']
        c = local_dict['c']
        if not density in stats_ours:
            stats_ours[density] = {
                'find': np.zeros((len(iters), len(channels))),
                'insert': np.zeros((len(iters), len(channels))),
                'activate': np.zeros((len(iters), len(channels)))
            }

            stats_stdgpu[density] = {
                'find': np.zeros((len(iters), len(channels))),
                'insert': np.zeros((len(iters), len(channels))),
            }

        stats_ours[density]['find'][
            map_iters_row[n], map_channels_col(c)] = local_dict['ours.find']
        stats_ours[density]['insert'][
            map_iters_row[n],
            map_channels_col(c)] = local_dict['ours.insertion']
        stats_ours[density]['activate'][
            map_iters_row[n], map_channels_col(c)] = local_dict['ours.activate']

        stats_stdgpu[density]['find'][
            map_iters_row[n], map_channels_col(c)] = local_dict['stdgpu.find']
        stats_stdgpu[density]['insert'][
            map_iters_row[n],
            map_channels_col(c)] = local_dict['stdgpu.insertion']

    colors = ['#ff000020', '#00ff0020', '#0000ff20', '#ffff0020']
    num_ops = [r'$10^3$', r'$10^4$', r'$10^5$', r'$10^6$']
    linestyles = ['solid', 'dotted', 'dashed', 'dashdot']
    markers = ['^', 's', 'x', 'o']
    fig, axes = plt.subplots(2, 2, figsize=(16, 9))

    densities = [0.1, 0.1, 0.99, 0.99]
    ops = ['insert', 'find', 'insert', 'find']

    for k in range(4):
        label_set = False
        for i in range(len(iters)):
            di = densities[k]
            opi = ops[k]
            ax = axes[k // 2, k % 2]

            if i == 3:
                limit = -3
            else:
                limit = None
            x = np.array(channels[:limit]) * 4
            stdgpu_curve = stats_stdgpu[di][opi][i][:limit]
            ours_curve = stats_ours[di][opi][i][:limit]

            # Color indicator
            if not label_set:
                ax.plot(x, ours_curve, color='b', label='ours')
                # linestyle=linestyles[i],
                #marker=markers[i])
                ax.plot(x, stdgpu_curve, color='r', label='stdgpu')
                #linestyle=linestyles[i],
                #marker=markers[i])
                label_set = True

            ax.plot(
                x,
                ours_curve,
                color='b',
                #linestyle=linestyles[i],
                marker=markers[i],
                label=num_ops[i])
            ax.plot(
                x,
                stdgpu_curve,
                color='r',
                #linestyle=linestyles[i],
                marker=markers[i])

            ax.fill(np.append(x, x[::-1]),
                    np.append(stdgpu_curve, ours_curve[::-1]),
                    color=colors[i])
            ax.set_title(r'Density = ${}$, Operation {}'.format(di, opi))
        ax.legend()
        ax.set_xlabel('Hashmap value size (byte)')
        ax.set_xscale('log')

        ax.set_ylabel('Time (ms)')
        ax.set_yscale('log')
        ax.grid()
    plt.tight_layout()
    plt.show()