"""
Usage:
$ python getNearest.py <input_context> <library_context_glob>
"""
import glob
import math
import sys


LIB_GLOB = '/Users/kristen/Dev/Toonchat/Avatar/kristen/testFit/DerivedData/roughFace/Build/Products/Debug/hair_contexts/*.txt'
INPUT_FILENAME = '/Users/kristen/Dev/Toonchat/Avatar/kristen/testFit/DerivedData/roughFace/Build/Products/Debug/input_shape_context.txt'


def GetShapeContexts(name, do_glob=False):
    if do_glob:
        globbed = glob.glob(name)
        all_lines = [open(file, 'r').readlines() for file in globbed]
        all_rows = [[line.split() for line in lines] for lines in all_lines]
        return all_rows
    else:
        input_file = open(name, 'r')
        lines = input_file.readlines()
        rows = [line.split() for line in lines]
        return rows


def GetShorterLen(x, y):
    len_x, len_y = len(x), len(y)
    if len_x < len_y:
        return len_x
    return len_y

def GetRowDistance(row1, row2):
    shorter_len = GetShorterLen(row1, row2)
    distance = 0.0
    for i in xrange(shorter_len):
        distance += abs(float(row1[i]) - float(row2[i]))
    return distance

def GetContextDistance(context1, context2):
    # [[.....],
    #  [.....],
    #    ...
    #  [.....]]
    # Match each row1 to each row2.
    shorter_len = GetShorterLen(context1, context2)
    print 'a', len(context1),  ': ', len(context2)
    match1, match2 = [], []
    i, j = None, None
    for i in xrange(len(context1)):
        row1 = context1[i]
        smallest_dist = -1
        smallest_row2 = None
        for j in xrange(len(context2)):
            row2 = context2[j]
            row_dist = GetRowDistance(row1, row2)
            if smallest_dist == -1 or smallest_dist < row_dist:
                smallest_dist = row_dist
                smallest_row2 = row2
        context2.pop(j)
        match1.append(row1)
        match2.append(smallest_row2)
        if len(match1) == shorter_len:
            break
        
    # Get distance.
    context_distance = 0.0
    for m1 in match1:
        for m2 in match2:
            context_distance += GetRowDistance(row1, row2)
    return context_distance


def FindNearestMatch(input_context, library_contexts):
    nearest_i = -1
    nearest_distance = -1
    distances = []
    print 'Number of library contexts: %s' % len(library_contexts)
    for i in xrange(len(library_contexts)):
        lib_context = library_contexts[i]
        distance = GetContextDistance(input_context, lib_context)
        distances.append(distance)
        if distance < nearest_distance or nearest_distance == -1:
            nearest_distance = distance
            nearest_i = i
    print 'Distances: %s' % distances
    return nearest_i

def main():
    library_contexts = GetShapeContexts(LIB_GLOB, do_glob=True)
    input_context = GetShapeContexts(INPUT_FILENAME)
    #library_contexts = GetShapeContexts(sys.argv[2], do_glob=True)
    #input_context = GetShapeContexts(sys.argv[1])

    # Compare input_context with every library_context to find
    #   closest match in library.
    nearest = FindNearestMatch(input_context, library_contexts)
    print 'NEAREST LIBRARY CONTEXT: %s' % nearest

if __name__ == '__main__':
    main()
