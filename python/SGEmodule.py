#!/bin/env python3
"""
filename: SGEmodule.cpp
created by: Jonah R. Huggins
created on: 25-04-10

description: 
Seperate function module, later turning into SGEmodule proper. 
"""
# -----------------------Package Import & Defined Arguements-------------------#
# Package Imports
import numpy as np

def SGEmodule(deterministic_values: np.ndarray):
    """
    Fake stochasticity being added to deterministic values
    """

    for index, value in enumerate(deterministic_values):

        random_choice_obj = np.random.default_rng()

        delta = random_choice_obj.random()

        sign = random_choice_obj.choice([-1, 1])

        deterministic_values[index] = value + (delta * sign)

    return deterministic_values

