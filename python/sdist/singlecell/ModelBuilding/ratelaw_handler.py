#!/bin/env python3 
# -*- coding: utf-8 -*-
"""
Author: JRH
Created: 05/24/2025
Description: parses string ratelaws and organizes by solver.
"""
import logging

import pandas as pd

logging.basicConfig(
    level=logging.INFO, # Overriden if Verbose Arg. True
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)


class Ratelaw:
    """Separates reaction differences without gratuitous if/else statements"""

    def __init__(self, reactionId: str, ratelaw: pd.Series):
        self.reactionId = reactionId
        self.ratelaw = ratelaw

        self.formula = None
        self.parameters = {'parameterId': [], 'value': []}
        self.reactants = None
        self.products = None
        self.compartment = ratelaw['compartment']

        self.__get_reactants_products()

        self.__get_rxn_formula()

        del self.reactionId
        del self.ratelaw

    def __get_reactants_products(self):
        """Parses reactants and products from 'r ; p' string in ratelaw row."""
        rxn = self.ratelaw.get('r ; p', '')

        if pd.isna(rxn) or not isinstance(rxn, str):
            logger.debug("Reaction entry is NaN or not a string: %s", rxn)
            return [], []

        reactants_str, products_str = (rxn.split(';') + [''])[:2]
        logger.debug("Parsed reaction string: reactants='%s', products='%s'", reactants_str, products_str)

        self.reactants = [r.strip() for r in reactants_str.split('+') if r.strip()]
        self.products = [p.strip() for p in products_str.split('+') if p.strip()]

        logger.debug("Final parsed lists: reactants=%s, products=%s", self.reactants, self.products)

    def __get_rxn_formula(self):
        """builds formula for non-mass-action ratelaws."""

        self.formula = self.ratelaw['ratelaw']