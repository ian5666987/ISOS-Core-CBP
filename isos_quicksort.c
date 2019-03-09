/* Base code taken from: https://www.geeksforgeeks.org/quick-sort/*/
/* Modified by Ian K for use in ISOS */
/* C implementation Isos_QuickSort ASC and DESC */

#include <stdio.h>
#include "isos.h"

// A utility function to swap two elements
void swap(struct IsosDueTask* a, struct IsosDueTask* b)
{
    struct IsosDueTask t;
    t = *a;
    *a = *b;
    *b = t;
}

/* This function takes last element as pivot, places
   the pivot element at its correct position in sorted
    array, and places all smaller (smaller than pivot)
   to left of pivot and all greater elements to right
   of pivot */
int partitionAsc (struct IsosDueTask arr[], int low, int high)
{
    unsigned char pivot;
    int i, j;

    pivot = arr[high].Priority;    // pivot
    i = (low - 1);  // Index of smaller element

    for (j = low; j <= high- 1; j++)
    {
        // If current element is smaller than or
        // equal to pivot
        if (arr[j].Priority <= pivot)
        {
            i++;    // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

/* This function takes last element as pivot, places
   the pivot element at its correct position in sorted
    array, and places all greater (greater than pivot)
   to left of pivot and all smaller elements to right
   of pivot */
int partitionDesc (struct IsosDueTask arr[], int low, int high)
{
    unsigned char pivot;
    int i, j;

    pivot = arr[high].Priority;    // pivot
    i = (low - 1);  // Index of smaller element

    for (j = low; j <= high- 1; j++)
    {
        // If current element is smaller than or
        // equal to pivot
        if (arr[j].Priority >= pivot)
        {
            i++;    // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

/* The main function that implements Isos_QuickSortAsc
 arr[] --> Array to be sorted,
  low  --> Starting index,
  high  --> Ending index */
void Isos_QuickSortAsc(struct IsosDueTask arr[], int low, int high)
{
    int pi;
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now
           at right place */
        pi = partitionAsc(arr, low, high);

        // Separately sort elements before
        // partition and after partition
        Isos_QuickSortAsc(arr, low, pi - 1);
        Isos_QuickSortAsc(arr, pi + 1, high);
    }
}

/* The main function that implements Isos_QuickSortDesc
 arr[] --> Array to be sorted,
  low  --> Starting index,
  high  --> Ending index */
void Isos_QuickSortDesc(struct IsosDueTask arr[], int low, int high)
{
    int pi;
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now
           at right place */
        pi = partitionDesc(arr, low, high);

        // Separately sort elements before
        // partition and after partition
        Isos_QuickSortDesc(arr, low, pi - 1);
        Isos_QuickSortDesc(arr, pi + 1, high);
    }
}
