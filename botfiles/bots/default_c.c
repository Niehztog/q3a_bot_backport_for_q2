// Default bot character file for Q3 botlib backport to Quake II.
// Loaded by BotLoadCharacter() via be_ai_char.c.
//
// Format: skill <level> { <index> <value> ... }
//
// Key characteristic indices (from ioq3/code/game/chars.h):
//   0  = name (string)
//   1  = gender (string: "male", "female", "it")
//   2  = attack_skill [0,1]
//   3  = weaponweights (string: filename)
//   4  = view_factor [0,1]
//   5  = view_maxchange [1,360]
//   6  = reactiontime [0,5]
//   7  = aim_accuracy [0,1]
//  16  = aim_skill [0,1]
//  21  = chat_file (string: filename)
//  22  = chat_name (string)
//  23  = chat_cpm [1,4000]
//  36  = croucher [0,1]
//  37  = jumper [0,1]
//  40  = itemweights (string: filename)
//  41  = aggression [0,1]
//  42  = selfpreservation [0,1]
//  44  = camper [0,1]
//  45  = easy_fragger [0,1]
//  46  = alertness [0,1]
//  47  = firethrottle [0,1]
//  48  = walker [0,1]

// Skill level 1 (beginner)
skill 1
{
    0   "DefaultBot"
    1   "male"
    2   0.2
    3   "bots/default_w.c"
    4   0.5
    5   90
    6   1.5
    7   0.3
    16  0.2
    21  "bots/default.c"
    22  "defaultbot"
    23  150
    24  0.1
    25  0.1
    26  0.1
    27  0.1
    28  0.1
    29  0.1
    36  0.2
    37  0.3
    38  0.0
    40  "bots/default_i.c"
    41  0.3
    42  0.7
    44  0.2
    45  0.5
    46  0.3
    47  0.8
    48  0
}

// Skill level 2
skill 2
{
    0   "DefaultBot"
    1   "male"
    2   0.4
    3   "bots/default_w.c"
    4   0.6
    5   120
    6   1.0
    7   0.5
    16  0.4
    21  "bots/default.c"
    22  "defaultbot"
    23  150
    24  0.2
    25  0.2
    26  0.2
    27  0.2
    28  0.2
    29  0.2
    36  0.2
    37  0.4
    38  0.2
    40  "bots/default_i.c"
    41  0.5
    42  0.5
    44  0.1
    45  0.5
    46  0.5
    47  0.7
    48  0
}

// Skill level 3
skill 3
{
    0   "DefaultBot"
    1   "male"
    2   0.6
    3   "bots/default_w.c"
    4   0.7
    5   150
    6   0.7
    7   0.65
    16  0.6
    21  "bots/default.c"
    22  "defaultbot"
    23  150
    24  0.3
    25  0.3
    26  0.3
    27  0.3
    28  0.3
    29  0.3
    36  0.3
    37  0.5
    38  0.5
    40  "bots/default_i.c"
    41  0.6
    42  0.4
    44  0.1
    45  0.5
    46  0.6
    47  0.6
    48  0
}

// Skill level 4
skill 4
{
    0   "DefaultBot"
    1   "male"
    2   0.8
    3   "bots/default_w.c"
    4   0.8
    5   180
    6   0.4
    7   0.8
    16  0.8
    21  "bots/default.c"
    22  "defaultbot"
    23  150
    24  0.4
    25  0.4
    26  0.4
    27  0.4
    28  0.4
    29  0.4
    36  0.4
    37  0.6
    38  0.7
    40  "bots/default_i.c"
    41  0.7
    42  0.3
    44  0.1
    45  0.6
    46  0.8
    47  0.5
    48  0
}

// Skill level 5 (expert)
skill 5
{
    0   "DefaultBot"
    1   "male"
    2   0.95
    3   "bots/default_w.c"
    4   0.95
    5   360
    6   0.2
    7   0.95
    16  0.95
    21  "bots/default.c"
    22  "defaultbot"
    23  150
    24  0.5
    25  0.5
    26  0.5
    27  0.5
    28  0.5
    29  0.5
    36  0.5
    37  0.7
    38  1.0
    40  "bots/default_i.c"
    41  0.85
    42  0.2
    44  0.05
    45  0.7
    46  0.95
    47  0.4
    48  0
}
