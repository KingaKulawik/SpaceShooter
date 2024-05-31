## Spis treści
1. [O projekcie](#o-projekcie)
2. [Funkcjonalności gry](#funkcjonalności-gry)
3. [Ekran gry](#ekran-gry)
4. [Wątki w programie](#wątki-w-programie)
5. [Sekcje krytyczne](#sekcje-krytyczne)
6. [Uruchomienie gry](#uruchomienie-gry)

## O projekcie
Założeniem projektu było stworzenie gry typu Galaxy Voyager, wyświetlającej się w terminalu i wykorzystującej do działania mechanizm wielowątkowości.
Gracz wciela się w pilota statku kosmicznego, którego zadaniem jest zebranie wszystkich cząsteczek antymaterii w galaktyce, jednocześnie unikając komet i gwiazd.

## Funkcjonalności gry
Podstawowa funkcjonalność gry opiera się na poruszaniu statkiem kosmicznym po galaktyce i zbieraniu cząsteczek antymaterii. 
Gracz musi unikać komet, które poruszają się po planszy.
Gracz wygrywa, gdy zbierze wszystkie cząsteczki antymaterii z planszy.
Gracz przegrywa, gdy zostanie trafiony przez kometę lub zderzy się z gwiazdą.

## Ekran gry
- Gra wyświetla się w terminalu za pomocą biblioteki ncurses.
- Statek gracza reprezentowany jest przez symbol `^`.
- Antymateria jest reprezentowana przez symbol `..`.
- Kometę reprezentuje symbol `*`.
- Gwiazdy strzelające kometami są reprezentowane przez symbol `O`.

## Wątki w programie
- **Statek kosmiczny (gracz)**
  - Wątek odpowiedzialny za poruszanie statkiem kosmicznym po planszy.
  - Obsługuje on wciśnięcia klawiszy przez gracza i na ich podstawie aktualizuje pozycję statku na planszy.
  - Wątek ten sprawdza również, czy statek nie trafił na kometę, co kończy grę.

- **Komet**
  - Każda gwiazda strzela kometami w losowych kierunkach.
  - Wątki te poruszają kometami po planszy, sprawdzając, czy kometa nie trafiła w statek kosmiczny lub nie zniknęła poza granicami planszy.
  - Jeśli kometa trafi statek, gra się kończy.

## Sekcje krytyczne
Kontrola nad sekcjami krytycznymi w grze jest realizowana za pomocą mutexów. 
Każda funkcja, która wchodzi do sekcji krytycznej, blokuje dostęp do zasobów wspólnych (np. planszy gry), aby inne funkcje nie mogły jednocześnie modyfikować tych zasobów. 
Jeśli funkcja chce wejść do sekcji krytycznej, ale zasoby są już zablokowane, zostaje wstrzymana do czasu zwolnienia zasobów.

## Uruchomienie gry
- **Ubuntu install ncurses**
  - sudo apt-get install libncurses5-dev libncursesw5-dev

- **Compilation**
  - g++ galaxy_voyager.cpp -o main -lncurses
  - g++ -std=c++11 galaxy_voyager.cpp -o main -lncurses -lpthread

- **Start**
  - ./main
