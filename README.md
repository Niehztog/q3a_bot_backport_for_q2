# Backport of the Quake III Arena Bot for Quake II

## Introduction

On December 8, 1998, Jan Paul "Mr. Elusive" van Waveren released the first version of his new artificial player for Quake II, called "[The Gladiator Bot](https://mrelusive.com/oldprojects/gladiator/gladiator.html)". In the coming months his bot grew in popularity, as it had modern artificial intelligence, making it more realistic and therefore superior to most other bots for Quake II.

The bot became so successful and popular, that even id software itself decided to hire its author Mr. Elusive to work together with their team on Quake III Arena and to port and advance the Gladiator Bot for [Quake III Arena](https://github.com/id-Software/Quake-III-Arena). It later became the base for the single player AI in the game.

Before moving to id software, Mr. Elusive even started porting his bot for other Quake Engine games like Half-Life and Sin, but that work unfortunately was never finished. There are still traces left of that attempt in the original source code files.

Mr. Elusive released the game source code for Gladiator Bot to the public, allowing mod developers to implement the Gladiator bot right into their mods. Unfortunately he released only the Quake II game source code part of his bot, keeping the source code of the bot library "botlib" private. The botlib contains the implementation of the artificial intelligence with map navigation, strategic behaviour and so on. This makes it unable to recompile his work for Quake II as a whole, making it difficult to run the Gladiator bot on modern Quake II engines and making it impossible to implement new features or bug fixes or to adapting it to new operating system architectures.

On January 31, 2017, Jan Paul "Mr. Elusive" van Waveren sadly passed away after suffering a severe sickness. May he rest in peace.

Before passing away, he of course finished his work of bringing the Gladiator Bot into Quake III, further perfecting its artificial intelligence and routing behaviour.

The whole Quake III Arena source code later was officially released by id software, containing all of Mr. Elusive's work.

## Technical Background

One speciality of the Gladiator Bot for Quake II is, that it incorporates not only the original game modes, but also both mission packs, Capture The Flag and even Rocket Arena II game modes into a single game library. From a narchitectural point of view this has many advantages over having it all in seperate libraries residing in different directories. One of those being the possibility to quickly switch games modes without loading a different mod or to share game assets between game modes. Quake III Arena uses a similar approach, as it also as Capture The Flag game mode incorporated into the base game.

As the Quake III Arena version of the former Gladiator Bot code has evolved over the time, the bot library from Quake III Arena is no longer compatible with the Quake II Gladiator bot, as it has been adapted to fit Quake III's game API and architecture.

## Goal of this project

The goal of this project is to backport the bot library files from Quake III Arena to work again the Gladiator Bot game library.
Advantages:
- As the Quake III Bots have significantly improved artificial intelligence, this would tremendously improve the single player experience of multiplayer game modes in Quake II compared to the original Gladiator Bot.
- Having the full source of the bot available it would be easy to port the bot to other operating systems or architectures or to modernized versions of the Quake II game engine like [Yamagi Quake II](https://github.com/yquake2/yquake2).
- The bot could as well be implemented into newer mods of the game or its features could be further advanced or improved.

## Current status

All relevant source code files have been identified and brought together in this repository. It was attempted to preserve the git history wherever possible, in order to make the evolution of the code more understandable.
An initial version of a Makefile for compilation has been added. The Makefile from Yamagi Q2 was used as a base for this.
Other than that, there is still a lot of work to do. While the Gladiator game and Q3 BotLib sources should more or less compile with several warnings, they are far from being compatible towards each other. A lot of work has to be done to make both work together.

## Technical approach

In order to make it more transparent of how this repository was setup, all necessary steps were documented and built into a [shell script](./setup.sh). This script downloads all necessary sources, extracts them and puts them together as they can be found in this repository. In case someone else wants to continue this work, he may use this script as a base to get started with.
The current analysis results of the code structure differences between Q2 Gladiator and Q3 Bots are documented in a [separate markdown file](./docs/DEVELOPMENT.md).

## How to compile

This repository and Makefile have been optimized to work with the Yamagi Q2 Build Environment. Follow [this guide](https://github.com/yquake2/yquake2/blob/master/doc/020_installation.md#compiling-from-source) on how to set up the build environment, clone this repo inside mingw32 shell, change to its directory and type `make`.

## Call for help

As I am rather inexperienced with C and the architecture of the Quake II or III game source, the goal of this project seems like a huge effort for me to accomplish on my own, if not nearly impossible without further help. So I would like to invite anyone with experiences in C and familiar with the Quake II source code to join me in the effort of bringing back this brilliant peace of Quake II history to modern engines.
Feel free to fork this repository and to make pull requests. You can contact me at `niehztog (at) gmail.com`.

January 29, 2023
