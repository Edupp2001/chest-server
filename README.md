Сервер запускается и ждет команды на старт игры

клиент настроен на локальный адрес в моей сети на момент написания, ввод вручную не предусмотрен

1 для запуска короткой

после чего создается detach поток, который по таймеру ждет подключения

по умолчанию игроки имеют 30 секунд на соединение

при подключении игрока ему выделяется еще один detach поток, в котором происходит вся логика взаимодействия

сервер не может выступать в роли игрока, хоть изначально данный функционал рассматривался, причиной отказа от функционала стал говнокод, который я банально устал дебажить

по этой же причине некоторые игроки могут не получить информацию о победителе

из-за сложности синхронизации данных между потоками, были использованы многие глобальные переменные

правила игры: https://lifehacker.ru/kak-igrat-v-sunduchok/

игроки спрашивают друг друга только по кругу

очередь строится итеративно через map <string ip, Player player>, то есть один и тот же набор игроков будет ходить в одном и том же порядке до тех пор пока не поменяется их айпи в локальной сети

кастомные IP не поддерживается, нужен сервер с ip 192.168.1.240 успешного запуска, доделывать уже скорее всего не буду
