# 🎯 ZAPPY  
**Year-End-Project**

## 📌 Description  
The goal of this project is to create a network game where several teams confront each other on
a tile map containing resources.
The winning team is the first one where at least 6 players reach the maximum elevation.
---

## 🚀 Compilation

### Using shell :
```bash
make zappy_server   # To compile the server
make zappy_gui      # To compile the graphical client
make zappy_ai       # To compile the ai client

make fclean     # To delete .o files and binaries
```

The binaries `zappy_server, zappy_gui, zappy_ai` will be generated at the root.

---

## 🧰 Binaries
```bash
./zappy_server -p port -x width -y height -n name1 name2 ... -c clientsNb -f freq
```
- `-p port`     :   port number.
- `-x width`    :   width of the world.
- `-y height`   :   height of the world.
- `-n  ...`     :   name of the team.
- `-c clientsNb`:   number of authorized clients per team.
- `f freq`      :   reciprocal of time unit for execution of actions.

```bash
./zappy_gui -p port -h machine
```
- `-p port`     :   port number.
- `-h machine`  :   hostname of the server.

```bash
./zappy_ai -p port -n name -h machine
```
- `-p port`     :   port number.
- `-n name`     :   name of the team.
- `-h machine`  :   name of the machine; localhost by default.
---

## ⚙️ Dependencies

- **C/C++ Standard Library**
- **Python3**
- **OpenGL**
- **SFML**

---

## 📁 Repository tree structure

---

## 🖹 Documentation

To get the complete project documentation please use Doxygen using the following command:

```bash
doxygen
```

Then go do the newly created `html` folder and open a server http

```Python
# exemple using python server
cd html && python -m http.server
```

---

## 🏗️ Architecture

![UML](<path to UML>)

---

## 👨‍💻 Authors & Credits

Project carried out as part of the B-YEP-400 module by:

[`Pierre-Etienne HENRY`](mailto:pierre-etienne.henry@epitech.eu) \
[`Ariles HARKATI`](mailto:ariles.harkat@epitech.eu) \
[`Ewen LAYLE EVENO`](mailto:ewen.layle-eveno@epitech.eu) \
[`Dylan ADGHAR`](mailto:dylan.adghar@epitech.eu) \
[`Benoit THOMAS`](mailto:benoit.thomas@epitech.eu)
