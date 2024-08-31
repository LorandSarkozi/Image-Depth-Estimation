# Depth Estimation from Stereo Images with Slanted Support Windows

This project implements a **depth estimation algorithm** from stereo images using **Slanted Support Windows**. It uses stereo pairs of images to compute disparity and depth maps, employing various refinement techniques. Built with **OpenCV** and **C++**, the algorithm provides a robust method for calculating depth from disparity.

## Features
- **Initial disparity hypotheses** based on Sum of Absolute Differences (SAD).
- **Multi-level slanted window refinement** for improved accuracy in disparity and depth estimation.
- **Generation of depth maps** from disparity maps.
- **Visualization** of disparity and depth maps using color mapping.
- **Performance comparison** with ground truth disparity maps for scoring.

## Overview

Depth estimation is performed using **stereo image pairs** (left and right). The core idea is to compute the disparity between corresponding pixels and use this disparity to derive depth information. **Slanted support windows** are utilized to refine the disparity estimation across various regions of the image.

## Algorithm Workflow

1. ### Initial Hypotheses Computation
    - **Function**: `computeInitialHypotheses()`
    - **Description**: This function calculates initial disparity hypotheses for each pixel in the left image using SAD. It searches over a range of disparities (up to `max_disparity`) and selects the best hypothesis for further refinement.

2. ### Disparity Hypothesis Refinement
    - **Function**: `computeHypotheses()`
    - **Description**: This function refines the disparity map by iteratively adjusting the window size across multiple scales, improving disparity accuracy in difficult regions.

3. ### Disparity Refinement with Slant
    - **Function**: `refineDisparityWithSlant()`
    - **Description**: This function improves disparity accuracy by accounting for slanted surfaces, enhancing depth estimation in complex geometry(still not finished).

4. ### Depth Map Creation
    - **Function**: `createDepthMap()`
    - **Description**: This function generates a depth map from the computed disparity values using known camera parameters (focal length and baseline).

5. ### Visualization
    - **Function**: `displayDisparityMap()`
    - **Description**: Normalizes and displays the disparity map.
    - **Function**: `createDepthMap()`
    - **Description**: Visualizes the depth map using color maps for enhanced interpretation.

6. ### Performance Scoring
    - **Function**: `computeScore()`
    - **Description**: Compares the computed disparity map with a ground truth map and calculates a performance score based on pixel accuracy.

## Dependencies

This project relies on the following libraries:

- **OpenCV**: For image processing, matrix operations, and visualizations.
- **C++ Standard Libraries**: For basic input/output and vector manipulations.
