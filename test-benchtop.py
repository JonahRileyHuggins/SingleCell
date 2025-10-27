import os
import sys
import logging

# Configure basic logging to a file
logging.basicConfig(filename='output.log', level=logging.DEBUG, 
                    format='%(asctime)s - %(levelname)s - %(message)s')

sys.path.append('./build')
sys.path.append('./python/sdist/singlecell/Benchtop/')
sys.path.append('./python/sdist/singlecell/Benchtop/src')
from benchtop.Experiment import Experiment
from wrappers.SingleCell import SingleCell
config_path = "benchmarks/TRAIL-time-to-death/TRAIL-time-to-death.yml"
experiment = Experiment(config_path, cores=os.cpu_count() , verbose=True)
## 2025-10-10 12:45:52,729 - INFO - Loading Experiment None details from /SingleCell/benchmarks/BIM-dependent-ERK-inhibition/BIM-dependent-ERK-inhibition.yml
## 2025-10-07 09:21:32,928 - INFO - Loading Experiment None details from /SPARCED/Benchtop/tests/BIM-dependent-ERK-inhibition/config.yml
experiment.run(SingleCell)
experiment.observable_calculation()
