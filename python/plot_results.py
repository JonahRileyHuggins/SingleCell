#!/bin/env python3

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import seaborn as sns
import matplotlib as mpl
import numpy as np
from matplotlib.lines import Line2D

data = pd.read_csv('../src/results.tsv', sep = '\t', header=0, index_col = False)

# Internal function
from unit_converter import nanomolar2mpc

headers = [
    'nuc_gene_a__LIGAND_',
    'nuc_gene_i__LIGAND_',
    'nuc_gene_a__RECEPTOR_',
    'nuc_gene_i__RECEPTOR_',
    'cyt_mrna__LIGAND_',
    'cyt_mrna__RECEPTOR_',
    'cyt_prot__LIGAND_',
    'cyt_prot__RECEPTOR_',
    'cyt_prot__LIGAND__RECEPTOR_'
]

data[headers[0]] = nanomolar2mpc(data[headers[0]], 1.75e-12)
data[headers[1]] = nanomolar2mpc(data[headers[1]], 1.75e-12)
data[headers[2]] = nanomolar2mpc(data[headers[2]], 1.75e-12)
data[headers[3]] = nanomolar2mpc(data[headers[3]], 1.75e-12)
data[headers[4]] = nanomolar2mpc(data[headers[4]], 1.75e-12)
data[headers[5]] = nanomolar2mpc(data[headers[5]], 1.75e-12)


fig = plt.figure(figsize=(15, 10))
gs = gridspec.GridSpec(3, 3, figure=fig, hspace=0.6, wspace=0.4)

# mpl.rcParams['font.size'] = 16
mpl.rcParams['font.weight'] = 'bold'
mpl.rcParams['axes.labelweight'] = 'bold'
mpl.rcParams['lines.linewidth'] = 3


stop = len(data.index)* 30.0
step = stop / len(data.index)

time = np.arange(0.0, stop, step)
start = time[0]
cyt_gene_a__ligand_init = data[headers[0]]
cyt_gene_a__receptor_init = data[headers[2]]
cyt_mrna_ligand_init = data[headers[4]][0]
cyt_mrna_receptor_init = data[headers[5]][0]
cyt_prot_ligand_init = data[headers[6]][0]
cyt_prot_receptor_init = data[headers[7]][0]

# === Top row (Bar plots comparing pairs) ===
ax1_0 = fig.add_subplot(gs[0, 0])
# ax1_0.plot(time[::100]/3600, data[headers[0]][::100], color='orange', label=headers[0])
ax1_0.plot(time[::100], data[headers[0]][::100], color='orange', label=headers[0])

# ax1_0.bar(time[::100]/3600, data[headers[1]][::100], color='blue', alpha=0.6, label=headers[1], width=0.5)
ax1_0.set_title("Ligand")
ax1_0.set_ylabel("Gene (mpc)")
ax1_0.set_xlabel('Time (hr.)')
# ax1_0.legend(frameon = False)

ax1_1 = fig.add_subplot(gs[0, 1])
# ax1_1.plot(time[::100]/3600, data[headers[2]][::100], color='cyan', label=headers[2])
ax1_1.plot(time[::100], data[headers[2]][::100], color='cyan', label=headers[2])

# ax1_1.bar(time[::100]/3600, data[headers[3]][::100], color='blue', alpha=0.6, label=headers[3], width=0.5)
ax1_1.set_title("Receptor")
ax1_1.set_ylabel("Gene (mpc)")
ax1_1.set_xlabel('Time (hr.)')
# ax1_1.legend(frameon = False, )

# === Middle row (Single line plots) ===
ax2_0 = fig.add_subplot(gs[1, 0])
ax2_0.plot(time/3600, data[headers[4]], color='orange')
ax2_0.set_ylabel("mRNA (mpc)")
ax2_0.set_xlabel('Time (hr.)')

ax2_1 = fig.add_subplot(gs[1, 1])
ax2_1.plot(time/3600, data[headers[5]], color='cyan')
ax2_1.set_ylabel("mRNA (mpc)")
ax2_1.set_xlabel('Time (hr.)')

# === Bottom row (Line plots) ===
ax3_0 = fig.add_subplot(gs[2, 0])  # Span full row
ax3_1 = fig.add_subplot(gs[2, 1])
ax3_2 = fig.add_subplot(gs[2, 2])
ax3_0.plot(time/3600, data[headers[6]], color='orange', label=headers[6])
ax3_0.set_ylabel("Protein (nM)")
ax3_0.set_xlabel('Time (hr.)')
ax3_1.plot(time/3600, data[headers[7]], color='cyan', label=headers[7])
ax3_1.set_ylabel("Protein (nM)")
ax3_1.set_xlabel('Time (hr.)')
ax3_2.plot(time/3600, data[headers[8]], color='#15b01a', label=headers[8])
ax3_2.set_ylabel("LIGAND:RECEPTOR Complex (nM)")
ax3_2.set_xlabel('Time (hr.)')

ax1_2 = fig.add_subplot(gs[0:2, 2])

ax1_2.text(-0.2, 1.0, "Simulation Settings: ", fontsize = 10)
ax1_2.text(0.0, 0.975, f"Start Time: {start} s", fontsize =7.5)
ax1_2.text(0.0, 0.95,f"Stop Time: {stop} s", fontsize = 7.5)
ax1_2.text(0.0, 0.925,f"Step Size: {step} s", fontsize = 7.5)

ax1_2.text(-0.2, 0.875, "Initial Conditions:", fontsize=7.5)
ax1_2.text(0.0, 0.85,f"cyt_mrna__LIGAND_: {cyt_mrna_ligand_init} (mpc)", fontsize = 7.5)
ax1_2.text(0.0, 0.825, "Ligand kTC: 0.005 s", fontsize=7.5)
ax1_2.text(0.0, 0.8, "Ligand kTCd: 0.00005 s", fontsize=7.5)
ax1_2.text(0.0, 0.775,f"cyt_mrna__RECEPTOR_: {cyt_mrna_receptor_init} (mpc)", fontsize = 7.5)
ax1_2.text(0.0, 0.75, " Receptor kTC: 0.005s", fontsize=7.5)
ax1_2.text(0.0, 0.725, "Receptor kTCd: 0.00005 s", fontsize=7.5)
ax1_2.text(0.0, 0.7, f"cyt_prot__LIGAND_: {cyt_prot_ligand_init} (nM)", fontsize=7.5)
ax1_2.text(0.0, 0.675, "Ligand kTL: 1.0 s", fontsize=7.5)
ax1_2.text(0.0, 0.65, "Ligand kTLd: 0.00005 s", fontsize=7.5)
ax1_2.text(0.0, 0.625, f"cyt_prot__RECEPTOR_: {cyt_prot_receptor_init} (nM)", fontsize=7.5)
ax1_2.text(0.0, 0.6, "Receptor kTL: 1.0", fontsize=7.5)
ax1_2.text(0.0, 0.575, "Receptor kTLd: 0.00005 s", fontsize=7.5)
ax1_2.text(0.0, 0.55, "LR-Complex kTLd: 0.00005 s", fontsize=7.5)

ax1_2.axis('off')

# Custom legend elements
legend_elements = [
    # Line2D([0], [0], marker='o', color='w', label='active', markerfacecolor='red', markersize=10),
    # Line2D([0], [0], marker='o', color='w', label='inactive', markerfacecolor='blue', markersize=10),
    Line2D([0], [0], marker='o', color='w', label='ligand', markerfacecolor='orange', markersize=10),
    Line2D([0], [0], marker='o', color='w', label='receptor', markerfacecolor='cyan', markersize=10),
    Line2D([0], [0], marker='o', color='w', label='L : R-Complex', markerfacecolor='#15b01a', markersize=10)
]

# Add legend manually to ax1_2
ax1_2.legend(handles=legend_elements, loc='lower left', bbox_to_anchor=(0, 0.10), frameon=False, fontsize=10)


sns.despine(fig)

# Final touches
# fig.suptitle("Gene Expression and Interaction Overview", fontsize=16, y=0.95)
plt.tight_layout(rect=[0, 0, 1, 0.95])
plt.savefig(fname = "LR-Model.png", dpi = 300)
plt.show()