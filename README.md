# gitalyze

Tool for analyzing changes in a git repository.

The purpose of this tool is to be able to fetch all commits from a git repository
and be able to create statistics in a per folder basis. Like generating the average
size or frequency of modifications recursively in the project tree.

It can generate it for the whole project lifetime or as a sequence of results to
have several datapoints throughout the lifetime of the project. By generating a
sequence of points, a chart can be drawn to be able to monitor whether the frequency
of changes increases exponentially in a specific module over time.
