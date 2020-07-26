#! /usr/bin/env python

"""Solve some tasks with A* and the LM-Cut heuristic."""

import os
import os.path
import platform

from downward.experiment import FastDownwardExperiment
from downward.reports.absolute import AbsoluteReport
from downward.reports.scatter import ScatterPlotReport
from lab import cached_revision
from lab.environments import BaselSlurmEnvironment, LocalEnvironment


ATTRIBUTES = ["coverage", "error", "expansions", "total_time", "expansions_until_last_jump", "search_time", "cost", "error", "run_dir"]

NODE = platform.node()
if NODE.endswith(".scicore.unibas.ch") or NODE.endswith(".cluster.bc2.ch"):
    # Create bigger suites with suites.py from the downward-benchmarks repo.
    SUITE = ["gripper"]
    ENV = BaselSlurmEnvironment(email="yannick.zutter@stud.unibas.ch")
else:
    SUITE = ["gripper"]
    ENV = LocalEnvironment(processes=2)
# Use path to your Fast Downward repository.
REPO = os.path.expanduser("~/CLionProjects/fast-downward")
BENCHMARKS_DIR = os.path.expanduser("~/benchmarks")
# If REVISION_CACHE is None, the default ./data/revision-cache is used.
REVISION_CACHE = os.environ.get("DOWNWARD_REVISION_CACHE")
VCS = cached_revision.get_version_control_system(REPO)
REV = "1639684f8d58bf79fa4d4c2f7f0685c0c867788f"

exp = FastDownwardExperiment(environment=ENV, revision_cache=REVISION_CACHE)

# Add built-in parsers to the experiment.
exp.add_parser(exp.EXITCODE_PARSER)
exp.add_parser(exp.TRANSLATOR_PARSER)
exp.add_parser(exp.SINGLE_SEARCH_PARSER)
exp.add_parser(exp.PLANNER_PARSER)

exp.add_suite(BENCHMARKS_DIR, SUITE)
exp.add_algorithm("astar blind default", REPO, REV, ["--search", "astar(blind(), sg=default)"])
exp.add_algorithm("astar blind naive", REPO, REV, ["--search", "astar(blind(), sg=naive)"])
exp.add_algorithm("astar blind relaxed", REPO, REV, ["--search", "astar(blind(), sg=relaxed)"])

# Add step that writes experiment files to disk.
exp.add_step("build", exp.build)

# Add step that executes all runs.
exp.add_step("start", exp.start_runs)

# Add step that collects properties from run directories and
# writes them to *-eval/properties.
exp.add_fetcher(name="fetch")

# Add report step (AbsoluteReport is the standard report).
exp.add_report(AbsoluteReport(attributes=ATTRIBUTES), outfile="report.html")

# Add scatter plot report step.
exp.add_report(
    ScatterPlotReport(attributes=["expansions"], filter_algorithm=["blind", "lmcut"]),
    outfile="scatterplot.png",
)

# Parse the commandline and show or run experiment steps.
exp.run_steps()