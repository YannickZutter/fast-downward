#! /usr/bin/env python

import os
import os.path
import platform

from downward.experiment import FastDownwardExperiment
from downward.reports.absolute import AbsoluteReport
from downward.reports.scatter import ScatterPlotReport
from lab import cached_revision
from lab.environments import BaselSlurmEnvironment, LocalEnvironment

ATTRIBUTES = ["total_time"]
#ATTRIBUTES = ["coverage", "expansions", "total_time", "expansions_until_last_jump", "search_time", "cost", "error"]

NODE = platform.node()
if NODE.endswith(".scicore.unibas.ch") or NODE.endswith(".cluster.bc2.ch"):
    # Create bigger suites with suites.py from the downward-benchmarks repo.
    SUITE = ["gripper:prob01.pddl"]
    REPO = os.path.expanduser("~/fast-downward")
    ENV = BaselSlurmEnvironment(email="yannick.zutter@stud.unibas.ch")
else:
    SUITE = ["gripper:prob01.pddl"]
    REPO = os.path.expanduser("~/CLionProjects/fast-downward")
    ENV = LocalEnvironment(processes=2)

# Use path to your Fast Downward repository.
BENCHMARKS_DIR = os.path.expanduser("~/benchmarks")
# If REVISION_CACHE is None, the default ./data/revision-cache is used.
REVISION_CACHE = os.environ.get("DOWNWARD_REVISION_CACHE")
VCS = cached_revision.get_version_control_system(REPO)
REV = "e3b60aa37949cabe885aabf3ce60c1eae495911c"

exp = FastDownwardExperiment(environment=ENV, revision_cache=REVISION_CACHE)

# Add built-in parsers to the experiment.
exp.add_parser(exp.EXITCODE_PARSER)
exp.add_parser(exp.TRANSLATOR_PARSER)
exp.add_parser(exp.SINGLE_SEARCH_PARSER)
exp.add_parser(exp.PLANNER_PARSER)

exp.add_suite(BENCHMARKS_DIR, SUITE)
exp.add_algorithm("default", REPO, REV, ["--search", "astar(blind(), sg = default)"])
exp.add_algorithm("naive", REPO, REV, ["--search", "astar(blind(), sg = naive)"])
exp.add_algorithm("marked", REPO, REV, ["--search", "astar(blind(), sg = marked)"])
exp.add_algorithm("timestamps", REPO, REV, ["--search", "astar(blind(), sg = timestamps)"])

# Add step that writes experiment files to disk.
exp.add_step("build", exp.build)

# Add step that executes all runs.
exp.add_step("start", exp.start_runs)

# Add step that collects properties from run directories and
# writes them to *-eval/properties.
exp.add_fetcher(name="fetch")

# Add report step (AbsoluteReport is the standard report).
exp.add_report(AbsoluteReport(attributes=ATTRIBUTES), outfile="report.html")

# Parse the commandline and show or run experiment steps.
exp.run_steps()
