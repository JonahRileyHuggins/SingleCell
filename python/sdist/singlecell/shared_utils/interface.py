#!/bin/env python3 
"""
author: Jonah R. Huggins
Created On: 2025-08-22

description: Abstract parent class for specifying simulator modules. 
Create a child class that has the bare minimum of each method described 
within the Parent.
"""
from abc import ABC, abstractmethod

class AbstractSimulator(ABC):

    def __init__(self, tool):
        """
        Enforced constructor that requires a user-supplied tool/template.
        
        Params:
        - tool : object
            The user's tool/interface instance.
        """
        self.tool = tool

    @abstractmethod
    def modify(self):
        """
        Abstract method that must be implemented in child classes.
        Intended to modify the tool or its configuration.
        """
        pass

    @abstractmethod
    def simulate(self, start, stop, step):
        """
        Abstract method that must be implemented in child classes.
        
        Parameters
        ----------
        start : float
            Simulation start time.
        stop : float
            Simulation stop time.
        step : float
            Simulation step size.
        """
        pass