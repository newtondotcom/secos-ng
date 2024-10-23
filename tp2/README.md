# TP2 - Les interruptions et les exceptions

Le but du TP est de bien comprendre les interruptions et les exceptions et
comment implémenter des gestionnaires.

## Rappels et prérequis théoriques

Niveau matériel, on a :

* Des informations stockées sur la pile lors de l’arrivée d’une interruption :
    * EFLAGS, CS, EIP, Error Code, seulement si pas de changement de niveau de
      privilèges
    * ESP et SS en supplément en cas de changement de niveau
* Le registre CPU IDTR à renseigner contenant une adresse à laquelle il
  s'attend trouver une table
* Une table de descripteurs d’interruption (IDT), pointeurs vers des routines
  à exécuter.

De son côté, l’OS doit au préalable :

* avoir implémenté des routines de traitement d’interruption
* les avoir référencées correctement dans une zone mémoire contenant une table
  de descripteurs d’interruption (IDT)
* avoir configuré le registre IDTR avec l'adresse de cette zone mémoire.

Dans ce TP, il pourra être utile de se référer à la documentation Intel pour
comprendre en détail ce que font les instructions suivantes et quel est leur
rôle : INT3, RET, IRET. Il pourra être également utile de savoir à quoi
correspond le numéro d'exception #6, ou savoir se servir des instructions
PUSH/PUSHA, POP/POPA.

## Un premier squelette

Notre noyau dispose déjà d'une petite IDT. Elle est configurée dans
[intr.c](../kernel/core/intr.c) par la fonction `intr_init()`,  qui 
a notamment permis d'intercepter les #GP du TP1.

L'IDT contient des descripteurs d'interruptions `int_desc_t` qui sont
initialisés avec leur propre handler d'interruption `isr`.

La valeur d'isr correspond à des petites fonctions que l'on peut voir comme
des trampolines, localisées dans [`idt.s`](../kernel/core/idt.s). Ils servent
à empiler le numéro de l'interruption et à aligner la pile lorsqu'il manque
un code d'erreur pour l'évènement survenu. Chaque trampoline, saute dans
`idt_common` qui appelle le gestionnaire d'interruptions de haut niveau de
notre noyau `intr_hdlr`.

## Notes sur le tracing

Ce TP est également l'occasion d'utiliser les options de "trace" de Qemu,
permettant de savoir ce qu'il se passe dans le CPU durant l'exécution de la
VM.

Dans le fichier [`config.mk`](../utils/config.mk), il est possible
d'ajouter `$(QDBG)` sur la ligne de définition de QOPT, afin de faire prendre
en compte les options de trace au lancement de Qemu. Par défaut, seules
quelques traces sont activées, elles permettent de voir les exceptions
générées.

## Prise en main de l'IDT

**Q1\* : Dans tp.c, localiser l'IDT et afficher son adresse de chargement**
  (cf. fonction `get_idtr()` définie dans [`segmem.h`](../kernel/include/segmem.h)).

## Gestion furtive des breakpoints #BP

L'idée à présent est de compléter le contenu de l'IDT actuelle, notamment pour
qu'elle puisse gérer l'exception #BP. Le but est de ne pas modifier
`intr_hdlr` mais d'intercepter les #BP en amont depuis [`tp.c`](./tp.c).

### Premier essai naïf : sous forme d'une simple fonction C

**Q2 : Dans [`tp.c`](./tp.c), commencer par écrire une routine de traitement
  dans une fonction, `bp_handler`, affichant un message de debug à
  l'écran.**

**Q3 : Modifier le descripteur d'interruption (cf. type `int_desc_t` défini 
  dans [`segmem.h`](../kernel/include/segmem.h)) de #BP, stocké
  dans l'IDT, afin d'y référencer `bp_handler()` à la place du trampoline
  déjà installé.**

**Q4 : Pour tester cette mise à jour, ajouter une fonction `bp_trigger`, dans
  [`tp.c`](./tp.c), déclenchant un breakpoint grâce à l'instruction `int3` et
  appeler `bp_trigger()` dans `tp()`.**

**Q5 : Cette implémentation pousse le CPU à générer une faute. Pour comprendre
  pourquoi, à l'aide d'un outil de désassemblage comme `objdump -D`, lister
  les instructions, générées à la compilation, de la fonction `bp_handler()`. 
  Quelle est la dernière instruction de cette fonction ? Quel est son
  impact sur la pile ? Est-ce cohérent avec ce qui était sur la pile au
  moment de l'arrivée d'une interruption ?**

  ```bash
  root@insa-21123:~/secos-ng/tp2# objdump -D kernel.elf | grep -A 18 "bp_handler"
  00303fca <bp_handler>:
    303fca:       55                      push   %ebp
    303fcb:       89 e5                   mov    %esp,%ebp
    303fcd:       83 ec 18                sub    $0x18,%esp
    303fd0:       8b 45 04                mov    0x4(%ebp),%eax
    303fd3:       89 45 f4                mov    %eax,-0xc(%ebp)
    303fd6:       83 ec 08                sub    $0x8,%esp
    303fd9:       ff 75 f4                push   -0xc(%ebp)
    303fdc:       68 86 48 30 00          push   $0x304886
    303fe1:       e8 ea f0 ff ff          call   3030d0 <printf>
    303fe6:       83 c4 10                add    $0x10,%esp
    303fe9:       83 ec 0c                sub    $0xc,%esp
    303fec:       68 a0 48 30 00          push   $0x3048a0
    303ff1:       e8 da f0 ff ff          call   3030d0 <printf>
    303ff6:       83 c4 10                add    $0x10,%esp
    303ff9:       90                      nop
    303ffa:       c9                      leave  
    303ffb:       c3                      ret   
  ```

  La dernière instruction de la fonction bp_handler est ret. Elle vide la pile (en faisant des pop). Le problème est qu'il manque l'adresse de retour.

### Deuxième essai : via l'assembleur inline

L'idée est de réécrire `bp_handler` en assembleur inline pour éviter l'écueil
de l'essai précédent.

**Q7 : **Au début de `bp_handler`, afficher la valeur stockée en `ebp-4` :**

```c
  uint32_t val;
   asm volatile ("mov 4(%%ebp), %0":"=r"(val));
```

**Quelle signification cette valeur a-t-elle ? S'aider à nouveau de `objdump -D`
pour comparer cette valeur à une adresse de votre noyau.**

L'asm volatile affiche la valeur de ebp-4 qui est là où est stocké l'adresse de retour. Donc la signification de val est l'adresse de retour.
Donc pour qu'on puisse y voir plus clair addr vaut 304d60 et val vaut 303f9b
Pour retrouver cette valeur dans le code kernel et voit ce à quoi elle correspond on fait : `objdump -D kernel.elf > q7` puis `cat q7 | grep 303f9b -B 5 -A 5` (B pour before et A pour after) : 
  ```bash
00303f97 <bp_trigger>:
  303f97:       55                      push   %ebp
  303f98:       89 e5                   mov    %esp,%ebp
  303f9a:       cc                      int3
  303f9b:       90                      nop  <------------------------ ebp-4 = saved EIP! Le nop sauve un peu le contexte je crois et c'est comme ça qu'on est censé savoir
  303f9c:       5d                      pop    %ebp
  303f9d:       c3                      ret
  ```

**Q8\* : Qu'est-ce qui n'est pas stocké par le CPU à l'arrivée d'une
  interruption et qu'il est impératif de sauvegarder avant tout traitement de
  l'interruption ? L'implémenter en assembleur inline dans  `bp_handler`.**

  La valeur de retour de la fonction bp_handler() ainsi que le contexte. 

**Q9\* : Par quelle instruction doit se terminer la routine pour que le noyau
  rende la main à la fonction tp() ? L'implémenter en assembleur inline dans
  `bp_handler`.**

  Ne pas oublier le leave -> revoir cours asm

**Q10 : Tester que le retour du traitement de l'interruption s'est effectué
  correctement en affichant un message de debug dans la fonction `bp_trigger()` 
  après le déclenchement du breakpoint.**

**Q11 : Quelles conclusions peut-on tirer du développement en C d'un
  gestionnaire d'interruption ? Pourquoi l'assembleur semble-t-il plus
  approprié ?**

  Je dirais que ça se fait en C mais comme il faut gérer la pile et tout et que cela se fait directement avec des instructions assembleur il vaut mieux le faire directement en assembleur.
