# Project solution ultimate backend

**This is the backend part for
the [Project solution ultimate](https://github.com/lucley64/project-solution-finder-ultimate/tree/main)**

## C++

The c++ part is a rest api coded with [cpprestsdk](https://github.com/microsoft/cpprestsdk)
and [sqlite3](https://www.sqlite.org/cintro.html)

### Compilation

**_Cmake 3.28 and ninja are required_**

#### Required: install cpprestsdk and sqlite3

1. cpprestsdk:

   If you have apt, run:
   ```Bash
      sudo apt update
      sudo apt install libcpprest-dev
   ```

2. sqlite3:

   Same for sqlite3:
   ```Bash
   sudo apt update
   sudo apt install sqlite3 libsqlite3-dev
   ```

To compile, simply execute this command:

```Bash
cmake --build ./cmake-build-debug --target project_solution_finder_ultimate_backend -j 6
```

### Running

To run the program, run the following command:

```Bash
./cmake-build-debug/project_solution_finder_ultimate_backend ./plateforme.db [--export]
```

The `--export` option is used to export all the solutions in a solutions.json file

## Python

Python 3.12

Create a python virtual environment using for example virtualenv

To check if virtualenv is already installed :

```Bash
pip list | grep virtualenv
```

If virtualenv does not appear, run :

```Bash
pip install virtualenv
```

Move to the directory containing project-solution-finder-ultimate-backend :

```Bash
cd path/to/project-solution-finder-ultimate-backend`
```

To create a virtual environment, execute the following command :

```Bash
python<version> -m venv .venv
```

Activate the virtual environment :

```Bash
source .venv/bin/activate
```

Download all required librairies :

```Bash
pip install -r requirements.txt
```

<div style="page-break-after: always; visibility: hidden"> </div>

# Citing & Authors of model

> reimers-2019-sentence-bert
> - Title : [Sentence-BERT: Sentence Embeddings using Siamese BERT-Networks](https://arxiv.org/abs/1908.10084)
> - Author : Reimers, Nils and Gurevych, Iryna
> - Booktitle : Proceedings of the 2019 Conference on Empirical Methods in Natural Language Processing
> - Month : November
> - Year : 2019
> - Publisher : Association for Computational Linguistics

> reimers-2020-multilingual-sentence-bert
> - Title : [Making Monolingual Sentence Embeddings Multilingual using Knowledge Distillation](https://arxiv.org/abs/2004.09813)
> - Author : Reimers, Nils and Gurevych, Iryna
> - Booktitle : Proceedings of the 2020 Conference on Empirical Methods in Natural Language Processing
> - Month : November
> - Year : 2020
> - Publisher : Association for Computational Linguistics