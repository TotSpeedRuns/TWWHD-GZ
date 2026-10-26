"""Check the public functional tables without game data or private inputs."""
from collections import Counter
from pathlib import Path
import hashlib
import json

HERE = Path(__file__).resolve().parent


def load(name):
    return json.loads((HERE / name).read_text(encoding="utf-8"))


def tuple_key(row):
    assert isinstance(row["stage"], str) and 1 <= len(row["stage"]) <= 8
    assert row["stage"].isascii()
    for field in ["room", "spawn", "layer"]:
        assert isinstance(row[field], int) and 0 <= row[field] <= 255
    return tuple(row[field] for field in ["stage", "room", "spawn", "layer"])


def main():
    plan = load("reference-plan.json")
    decisions = load("decisions.json")
    tables = load("normalized-tables.json")
    pins = load("source-pins.json")
    assert plan["source_blob"] == "3aa4fc3a192e51c78c2ec9cf992d11e890202c96"
    assert len(plan["rows"]) == len(decisions["rows"]) == 174
    assert [row["line"] for row in plan["rows"]] == [row["line"] for row in decisions["rows"]]
    assert len({row["line"] for row in decisions["rows"]}) == 174
    assert Counter(row["status"] for row in decisions["rows"]) == {
        "existing": 76, "supported": 86, "excluded": 9, "contextual": 3}
    assert len(tables["warps"]) == 396 and len(tables["folders"]) == 63
    assert len(tables["quadrants"]) == 49 and len(tables["destinations"]) == 218
    assert [row["id"] for row in tables["warps"]] == list(range(396))
    tuples = [tuple_key(row) for row in tables["warps"]]
    assert len(set(tuples)) == 220
    for planned, row in zip(plan["rows"], decisions["rows"]):
        assert row["evidence"] and planned["reference_label"] == row["reference_label"]
        if row["status"] not in ["existing", "supported"]:
            assert row["reason"]
            continue
        assert tuple_key(row) in tuples
        assert row["stage"] != "Kaze"
        if row["stage"] == "PShip":
            assert planned["sector"] is None and row["sector"] is None
    for row in tables["folders"]:
        assert 0 <= row["start"] and 0 <= row["count"]
        if row["kind"] == 0:
            assert row["start"] + row["count"] <= len(tuples)
            selected = tuples[row["start"]:row["start"] + row["count"]]
            assert len(selected) == len(set(selected))
    assert tables["folders"][2]["label"] == "Hyrule"
    assert tables["folders"][2]["count"] == 21
    for row in tables["quadrants"]:
        selected = tables["destinations"][row["start"]:row["start"] + row["count"]]
        assert row["start"] + row["count"] <= len(tables["destinations"])
        selected_tuples = [tuples[entry["warp"]] for entry in selected]
        assert len(selected_tuples) == len(set(selected_tuples))
        assert not any(key[0] == "PShip" for key in selected_tuples)
    repairs = decisions["current_repairs"]
    assert [row["warp"] for row in repairs] == [78, 101, 105]
    for row in repairs:
        assert tuple_key(row["before"]) != tuple_key(row["after"])
        assert tuple_key(row["after"]) == tuples[row["warp"]]
    assert tables["warps"][78]["label"] == "Triforce Cave"
    assert tuples[78] == tuples[79]
    for name, expected in pins["published_files"].items():
        data = (HERE / name).read_bytes()
        assert len(data) == expected["bytes"] and hashlib.sha256(data).hexdigest() == expected["sha256"]
    print(json.dumps(dict(passed=True, reference_rows=174, unique_tuples=220,
                         physical_records=396, chart_destinations=218,
                         folders=63, Hyrule_entries=21, current_repairs=3,
                         game_or_private_build_inputs_used=False,
                         native_gameplay_qualified=False)))


if __name__ == "__main__":
    main()
