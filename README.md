# Jom-and-Terry
Jom and Terry e un proiect in care am implementat un server concurent ce accepta maxim 4 perechi de clienti si le trimite mesaje la fiecare 3 secunde daca indeplinesc o conditie


Sa se implementeze un server TCP concurent care rezolva conflictul dintre maxim 4 perechi de clienti - identificati prin numele Jom si, respectiv, Terry - in felul urmator. Daca se conecteaza primul un client Terry, atunci va primi o bila magica, in caz contrar o va primi un client Jom. Clientul care poseda bila magica va trimite din 3 in 3 secunde (prin intermediul serverului) tuturor celorlalti clienti cate un mesaj de avertisment. Mesajul va fi ignorat de clientii care poseda la randul lor bila magica. Daca un client primeste de trei ori cate un mesaj de avertisment, va fi deconectat.
