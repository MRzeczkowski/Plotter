/*!

\mainpage Programowanie aplikacji klient-serwer - Wykresy próbek

\authors Mateusz Rzeczkowski
\authors Krzysztof Kozoń

\date 11.07.2018

\details Aplikacja składa się z klienta i serwera napisanych w C. Klient wysyła do serwera plik zawierający próbki pobierane z urządzenia pomiarowego w równych
odstępach czasu (parametr konfigurowalny). Serwer na tej podstawie tworzy wykres pomiarów
wykorzystując aplikację do rysowania (np. gnuplot), eksportuje go do pliku i przesyła klientowi.
Komunikacja zabezpieczona jest za pomocą OpenSSL.

\section s1 Dokumentacja użytkownika
	\subsection ss1 Serwer
	Należy uruchomić serwer poprzez uruchomienie pliku ./PLOTTER_SERVER z konsoli systemowej. Po uruchomieniu sie, serwer poprosi o haslo certyfikatu, ktore należy podac. W przypadku bledu nalezy sie upewnic czy wpisywane haslo jest poprawne.

	\subsection ss2 Klient
	Należy uruchomić klienta poprzez uruchomienie pliku ./PLOTTER_CLIENT z konsoli systemowej, dodatkowo jako argument można podać ścieżkę do pliku, który chcemy przeslac (domyślnie plik nie wiekszy niz 8kB). Po przeslaniu pliku serwer odpowie odsylajac wykres.


*/
