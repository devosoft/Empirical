"""Exercise the example executable, using one-sided dice for exact expectations."""
import pathlib
import subprocess
import sys

program = pathlib.Path(sys.argv[1]).resolve()


def run(*args, ok=True):
    result = subprocess.run([str(program), *args], text=True, capture_output=True, timeout=10)
    assert result.returncode == (0 if ok else 1), (args, result.stdout, result.stderr)
    if ok:
        assert not result.stderr, (args, result.stderr)
    else:
        assert result.stderr, args
        assert not result.stdout, (args, result.stdout)
    return result.stdout.strip()


assert "Usage: dice" in run("--help")
assert subprocess.run([str(program)], capture_output=True).returncode == 1
for args, expected in [
    (("42",), 42),
    (("0",), 0),
    (("d1",), 1),
    (("d1 - d1 + 10",), 10),
    (("3d1 + d1 - 2",), 2),
    (("d 1",), 1),
    (("20", "-", "5", "-", "3"), 12),
    (("20-5+3",), 18),
    (("2 - 5 - 3",), -6),
    (("3d1", "-", "2d1", "+", "10"), 11),
    (("  3 d 1\t- 2d1 + 10\n",), 11),
    (("0d6 + 10",), 10),
    (("100000d1",), 100000),
    (("9223372036854775807",), 9223372036854775807),
    (("0 - 9223372036854775807 - 1",), -9223372036854775808),
]:
    assert int(run(*args)) == expected, args

# Check bounds, not particular random values or a statistical distribution.
for _ in range(10):
    assert 5 <= int(run("3d6", "-", "2d4", "+", "10")) <= 26
    assert 1 <= int(run("1d1000000")) <= 1000000
    assert 1 <= int(run("d6")) <= 6

for expression in [
    "", " ", "d", "d0", "dd6", "d1000001", "3d", "3d0", "1d1000001", "100001d6", "-2",
    "3d6 +", "3d6 * 2", "(3d6)", "3.5", "2 3", "1 + + 2", "1d6garbage",
    "9223372036854775808", "999999999999999999999999999999d6",
    "9223372036854775807 + 1", "0 - 9223372036854775807 - 2",
]:
    run(expression, ok=False)

print("Dice example checks passed.")
