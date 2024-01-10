import numpy as np
import scipy.stats as stats

def calculate_std_conf(samples):
    # Calculate the standard deviation
    std = np.std(samples)
    
    # Calculate the confidence interval
    confidence_level = 0.95
    degrees_freedom = len(samples) - 1
    sample_mean = np.mean(samples)
    sample_standard_error = stats.sem(samples)

    confidence_interval = stats.t.interval(confidence_level, degrees_freedom, sample_mean, sample_standard_error)
    
    return std, confidence_interval