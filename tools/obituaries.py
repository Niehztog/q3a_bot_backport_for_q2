#!/usr/bin/env python3
"""The obituaries of game_q2/p_client.c ClientObituary as botlib match
templates, the way Gladiator's botlib read deaths from its bots' consoles
(BotMatchMessage, MSG_DEATH).

    tools/obituaries.py templates   the MTCONTEXT_CLIENTOBITUARY block of
                                    assets/botfiles/match.c
    tools/obituaries.py check       that block is up to date, and every line
                                    ClientObituary can print matches the
                                    template that means it (a model of the
                                    botlib's StringsMatch, be_ai_chat.c)
    tools/obituaries.py cases       those lines, tab separated: text, kind,
                                    means of death, victim, killer

ClientObituary prints "victim message." for a death by the world (its first
switch) or by one's own hand (the `attacker == self` switch, which overrides
the first) and "victim message killer message2" for a kill (the switch under
`attacker->client`), picking at random among up to 16 messages, some
depending on the victim's gender; "victim died." when none applies. A
template's type is the kind (MSG_WORLDDEATH, MSG_SELFDEATH, MSG_DEATH), its
sub type the means of death (MOD_*, assets/botfiles/mod.h). Where several
means share a message, the one deathmatch meets is named (PREFER).
"""

import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
SOURCE = REPO / "game_q2" / "p_client.c"
MATCHFILE = REPO / "assets" / "botfiles" / "match.c"
BEGIN = "MTCONTEXT_CLIENTOBITUARY\n{"
END = "} //end MTCONTEXT_CLIENTOBITUARY"

GENDERS = ("male", "female", "neutral")
# a message shared by several means of death: the one deathmatch meets
PREFER = {("EXPLOSIVE", "BARREL"): "BARREL",
          ("BOMB", "SPLASH", "TRIGGER_HURT"): "TRIGGER_HURT",
          ("HG_SPLASH", "G_SPLASH"): "G_SPLASH",
          ("OTHER",): "SUICIDE"}
KINDS = {"world": "MSG_WORLDDEATH", "self": "MSG_SELFDEATH", "kill": "MSG_DEATH"}

ASSIGN = re.compile(r'\b(message2?)\[(\d+)\]\s*=\s*"((?:[^"\\]|\\.)*)"\s*;')
LABEL = re.compile(r"^\s*(?:case\s+MOD_(\w+)|(default))\s*:")


def function_body():
    text = SOURCE.read_text(errors="replace")
    body = text[text.index("void ClientObituary"):]
    body = body[:body.index("\n}\n")]
    # a misspelt #ifdef (XAXTIX) that never compiles
    return re.sub(r"#ifdef XAXTIX.*?#endif[^\n]*", "", body, flags=re.S)


def parse_switch(text):
    """[(labels, [(gender or None, var, index, string)])] of one switch."""
    groups, labels, stmts = [], [], []
    depth, pending, cond, cond_depth = 0, None, None, None
    for line in text.splitlines():
        code = line.split("//")[0].strip()
        m = LABEL.match(line)
        if m:
            if stmts:
                groups.append((labels, stmts))
                labels, stmts = [], []
            labels.append(m.group(1) or "OTHER")
            continue
        if code.startswith("if (IsNeutral(self))"):
            pending = "neutral"
        elif code.startswith("else if (IsFemale(self))"):
            pending = "female"
        elif code == "else":
            pending = "male"
        elif code.startswith("{"):
            depth += 1
            if pending:
                cond, cond_depth, pending = pending, depth, None
        elif code.startswith("}"):
            if cond and depth == cond_depth:
                cond = None
            depth -= 1
        a = ASSIGN.search(code)
        if a:
            stmts.append((pending or cond, a.group(1), int(a.group(2)), a.group(3)))
            pending = None
    if stmts:
        groups.append((labels, stmts))
    return groups


def switches():
    body = function_body()
    i_self = body.index("if (attacker == self)")
    i_kill = body.index("if (attacker && attacker->client)", i_self)
    return (parse_switch(body[:i_self]), parse_switch(body[i_self:i_kill]),
            parse_switch(body[i_kill:]))


def apply(group, gender, message, message2):
    for cond, var, k, s in group[1]:
        if cond in (None, gender):
            (message if var == "message" else message2)[k] = s


def means(labels):
    return "MOD_" + PREFER.get(tuple(labels), labels[0])


def group_of(groups, mod, default=False):
    for g in groups:
        if mod in g[0]:
            return g
    return next((g for g in groups if "OTHER" in g[0]), None) if default else None


def templates():
    """[(kind, mod, message, message2)] in match file order, no repeats."""
    world, own, kill = switches()
    out, seen = [], {}
    for kind, groups in (("world", world), ("self", own), ("kill", kill)):
        for group in groups:
            for gender in GENDERS:
                message, message2 = [None] * 16, [""] * 16
                apply(group, gender, message, message2)
                for k, m in enumerate(message):
                    if m is None:
                        continue
                    key = (m, message2[k] if kind == "kill" else "", kind == "kill")
                    t = (kind, means(group[0])) + key[:2]
                    if key in seen:
                        if seen[key] != t:
                            sys.exit(f"obituaries: {key} means {seen[key]} and {t}")
                        continue
                    seen[key] = t
                    out.append(t)
    out.append(("world", "MOD_UNKNOWN", "died", ""))
    return out


def quote(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def block():
    lines = [BEGIN]
    last = None
    for kind, mod, m, m2 in templates():
        if (kind, mod) != last:
            lines.append({"world": "\t//killed by the world: ",
                          "self": "\t//killed by oneself: ",
                          "kill": "\t//killed by KILLER: "}[kind] + mod)
            last = (kind, mod)
        if kind == "kill":
            pieces = ["VICTIM", quote(f" {m} "), "KILLER"] + ([quote(m2)] if m2 else [])
        else:
            pieces = ["VICTIM", quote(f" {m}.")]
        lines.append(f"\t{', '.join(pieces)} = ({KINDS[kind]}, {mod});")
    lines.append(END)
    return "\n".join(lines)


def cases():
    """Every line ClientObituary prints, as (text, kind, mods, victim,
    killer): each means of death, gender and attacker (the world, the victim
    itself, another player). Drowning and lava are the world's (P_WorldEffects):
    the victim is never their attacker, which would leave some of their first
    switch messages under the own-hand ones."""
    world, own, kill = switches()
    mods = sorted({l for gs in (world, own, kill) for g in gs for l in g[0]} - {"OTHER"})
    names = [("Ranger", "Visor"), ("Sir Kills-a-lot", "Mr. X"), ("A", "B's")]
    out = {}
    for mod in mods + ["UNKNOWN"]:
        for gender in GENDERS:
            for attacker in ("world", "self", "player"):
                if attacker == "self" and mod in ("WATER", "LAVA"):
                    continue
                message, message2 = [None] * 16, [""] * 16
                g = group_of(world, mod)
                if g:
                    apply(g, gender, message, message2)
                if attacker == "self":
                    g = group_of(own, mod, default=True)
                    apply(g, gender, message, message2)
                if message[0] is not None:
                    n = message.index(None) if None in message else 16
                    for victim, _ in names:
                        for i in range(n):
                            key = (f"{victim} {message[i]}.",
                                   "self" if attacker == "self" else "world", victim, "")
                            out.setdefault(key, set()).add("MOD_" + mod)
                    continue
                if attacker == "player":
                    g = group_of(kill, mod)
                    if g:
                        apply(g, gender, message, message2)
                    if message[0] is not None:
                        n = message.index(None) if None in message else 16
                        for victim, killer in names:
                            for i in range(n):
                                key = (f"{victim} {message[i]} {killer}{message2[i]}",
                                       "kill", victim, killer)
                                out.setdefault(key, set()).add("MOD_" + mod)
                        continue
                for victim, _ in names:
                    out.setdefault((f"{victim} died.", "world", victim, ""), set()).add("MOD_" + mod)
    return [(t, kind, mods, v, k) for (t, kind, v, k), mods in out.items()]


def strings_match(pieces, string):
    """be_ai_chat.c StringsMatch: a piece after a variable is found where it
    first occurs (any case), any other piece must follow at once, and the
    line must be used up unless it ends in a variable."""
    variables, last, pos = {}, None, 0
    for piece in pieces:
        if isinstance(piece, int):
            variables[piece], last = [pos, None], piece
            continue
        index = string.upper().find(piece.upper(), pos)
        if index < 0 or (last is None and index != pos):
            return None
        if last is not None:
            variables[last][1] = index - variables[last][0]
            last = None
        pos = index + len(piece)
    if last is not None:
        variables[last][1] = len(string) - variables[last][0]
    elif pos != len(string):
        return None
    return {v: string[o:o + n] for v, (o, n) in variables.items()}


def check():
    table = [(kind, mod, ([0, f" {m} ", 1] + ([m2] if m2 else [])) if kind == "kill"
              else [0, f" {m}."]) for kind, mod, m, m2 in templates()]
    bad = shared = 0
    for text, kind, mods, victim, killer in cases():
        found = next(((k, mod, v) for k, mod, pieces in table
                      for v in [strings_match(pieces, text)] if v is not None), None)
        if not found:
            print(f"no template: {text!r}")
            bad += 1
            continue
        k, mod, v = found
        if (k, v.get(0), v.get(1, "")) != (kind, victim, killer) or mod not in mods:
            print(f"wrong: {text!r} -> {k} {mod} {v}, printed as {kind} {sorted(mods)}")
            bad += 1
        shared += len(mods) > 1
    n = len(cases())
    print(f"obituaries: {n} lines, {len(table)} templates, {bad} wrong; "
          f"{shared} lines are printed for more than one means of death")
    text = MATCHFILE.read_text(errors="replace").replace("\r\n", "\n")
    if block() not in text:
        print(f"obituaries: the MTCONTEXT_CLIENTOBITUARY block of {MATCHFILE} is not "
              "the one game_q2/p_client.c gives (tools/obituaries.py templates)")
        return 1
    return 1 if bad else 0


if __name__ == "__main__":
    mode = sys.argv[1] if len(sys.argv) > 1 else "check"
    if mode == "templates":
        print(block())
    elif mode == "cases":
        for text, kind, mods, victim, killer in cases():
            print("\t".join([text, kind, ",".join(sorted(mods)), victim, killer]))
    elif mode == "check":
        sys.exit(check())
    else:
        sys.exit(__doc__)
