#!/bin/env python3

import argparse
import pyperclip

parser = argparse.ArgumentParser(prog = "unit-converter")
parser.add_argument("-m", "--mpc", help="target unit molecules / cell")
parser.add_argument("-n", "--nanomolar", help = "target unit nanomolar")
parser.add_argument("-c", "--compartment_volume", help="volume (liters) of cellular compartment" \
                    "for unit conversion")

AVOGADRO = 6.022e23

def mpc2nanomolar(mpc_value: float, compartment_volume: float)-> float:
    """converts molecules per cell to nanoMolar value
    
    Parameters:
    - mpc_value (float): number of molecules in cell.
    - compartment_volume (float): compartmental volume, needed for conversion

    Returns:
    - nanomolar_value (float): 10e-9 molar concentration (nanomoles per liter) value
    """
    return (mpc_value * (1.0 / compartment_volume) * (1.0 / AVOGADRO) * 1e9)


def nanomolar2mpc(nanomolar_value: float, compartment_volume: float) -> float:
    """converts nanoMolar values to molecule per cell.
    
    Parameters:
    - nanomolar_value (float): concentration (nanomoles per liter) of species
    - compartment_volume (float): compartmental volume, needed for conversion

    Returns:
    - mpc_value (float): discrete number of molecules per cell volume
    """
    return (nanomolar_value * (1.0 / 1.0e9) * AVOGADRO * (compartment_volume/1.0))



if __name__ == '__main__':

    args = parser.parse_args()
    if args.nanomolar:
        out = mpc2nanomolar(float(args.nanomolar), float(args.compartment_volume))
        print(out, " nM")

    elif args.mpc:
        out = nanomolar2mpc(float(args.mpc), float(args.compartment_volume))
        print(out, " molecules / cell")

    else:
        out = None


    # Copy to clipboard:
    pyperclip.copy(out)


