<?php

#	Developer: AgArtY M.C. (Vladimir Vladimirovich)                         #
#	E-mail: amc@agarty-a.ru                                                 #
#	Site: http://www.agarty-a.ru                                            #
#	                                                                        #
#	Copying, distribution, sale of an initial code without the written      #
#	approval of the developer is forbidden.                                 #


class func {

    /**
     * Возвращает размер в байтах исходя из переданных данных
     *
     * echo func::filesize_title(1024); //1 КБ
     * echo func::filesize_title(1024*1024*5); //5 МБ
     * echo func::filesize_title(1024*1024*1024*100); //100 ГБ
     * echo func::filesize_title(1); //1 Байт
     *
     *
     * @param int $total_size
     * @return string
     */
    final static public function filesize_title($total_size=0) {

        if ($total_size > 1024*1024*1024*1024) {
            return round((((($total_size)/1024)/1024)/1024)/1024, 2).' '.lang::txt('size_tb');
        }
        elseif ($total_size > 1024*1024*1024) {
            return round(((($total_size)/1024)/1024)/1024, 2).' '.lang::txt('size_gb');
        }
        else if ($total_size > 1024*1024) {
            return round((($total_size)/1024)/1024, 2).' '.lang::txt('size_mb');
        }
        else if ($total_size > 1024) {
            return round(($total_size)/1024, 2).' '.lang::txt('size_kb');
        }
        else if ($total_size > 0){
            return $total_size.' '.lang::txt('size_byte');
        }
        else return "0";

    }


    /**
     * $attr = array(
     *      'total' => 200,
     *      'limit' => 10,
     *      'nowPage' => 3,
     *      'nameGET' => 'page',
     *      'pfx' => '?',
     *      'uri' => $_SERVER['REQUEST_URI'],
     *      'step' => 8,
     *      'maxPages' => 10,
     *      'tmpl' => array(
     *          'next' => '<a href="{path}">следующая</a>',
     *          'prev' => '<a href="{path}">предыдущая</a>',
     *          'page' => '<a href="{path}">{page}</a>',
     *          'pageActive' =>'<a href="{path}" class="pageActive">{page}</a>',
     *          'sp' => ' | '
     *      )
     * );
     *
     * @param array $attr
     * @return stdClass
     */
    final static public function pages(array $attr) {
        $return = new stdClass();

        //Убираем из адресной строки указатель страницы
        $nameGET = preg_replace('#[^0-9a-zA-Z_]#', null, $attr['nameGET']);
        if (!$nameGET) $nameGET = 'page';
        if (!$attr['uri']) $attr['uri'] = $_SERVER['REQUEST_URI'];
        $parseURI = parse_url($attr['uri']);

        //Строим запрос $_GET, пропуская nameGET
        $GET = http_build_query($_GET);
        if ($nameGET) {
            $GET = preg_replace("#[\?&]?{$nameGET}=[^&]+#", null, $GET);
        }

        //Убираем последний & и строим полный путь
        $uri = $parseURI['path'].($GET ? '?'.$GET : null);

        $limit = func::numeric($attr['limit']);
        if (!$limit || $limit < 1) $limit = 1;

        $total = func::numeric($attr['total']);
        $nowPage = func::numeric($attr['nowPage']);
        $maxPages = func::numeric($attr['maxPages']);
        $step = func::numeric($attr['step']);

        if (!$maxPages || $maxPages < 1) $maxPages = 10;
        if (!$step || $step < 1) $step = 8;

        //Всего страниц, округляем в большую сторону
        $pages = ceil( $total/$limit );

        //Если мы находимся на какой-либо странице, то вычисляем, сколько выбрать
        if ($nowPage && $nowPage > 0) {
            //Что-то вроде: LIMIT 1000, 20
            $return->limit = ($limit * $nowPage).', '.$limit;
        } else {
            $return->limit = $limit;
            $nowPage = 0;
        }

        //Назначаем префикс к nameGET
        $pfx = $attr['pfx'] == '&' ? '&' : '?';
        $pfx = strstr($uri, '?') ? '&' : $pfx;

        $prev_page = $nowPage < 1 ? $pages-1 : $nowPage - 1;
        $next_page = ($nowPage + 1) >= $pages ? 0 : $nowPage + 1;

        //Шаблон "предыдущая страница"
        $prevPATH = $uri.$pfx.$nameGET.'='.$prev_page;
        $prevTMPL = $attr['tmpl']['prev'] ? ($attr['tmpl']['prev'] !== 'NO' ? $attr['tmpl']['prev'] : null) : '<a href="{path}">Prev</a>';
        $prevLINK = str_replace('{path}', $prevPATH, $prevTMPL);

        //Шаблон "следующая страница"
        $nextPATH = $uri.$pfx.$nameGET.'='.$next_page;
        $nextTMPL = $attr['tmpl']['next'] ? ($attr['tmpl']['next'] !== 'NO' ? $attr['tmpl']['next'] : null) : '<a href="{path}">Next</a>';
        $nextLINK = str_replace('{path}', $nextPATH, $nextTMPL);

        //Шаблоны страниц
        $pageTMPL = $attr['tmpl']['page'] ? $attr['tmpl']['page'] : '<a href="{path}">{page}</a>';
        $pageActiveTMPL = $attr['tmpl']['pageActive'] ? $attr['tmpl']['pageActive'] : '<a href="{path}" class="active">{page}</a>';

        //Если общее кол-во больше лимита
        if ($total > $limit) {
            //Если страниц больше
            if ($pages > $maxPages) {
                //Если есть текущая страница, и она не равна нулю, а так же она больше либо равно шагу
                if ($nowPage && $nowPage > 0 && $nowPage >= $step) {
                    $step2 = ($step - 1)/2;
                    $p_start = $nowPage - $step2;
                    $p_end = $nowPage + $step2;
                } else {
                   $p_start = 0;
                   $p_end = $step;
                }

                //Все, дальше пиндец. Как придумал в 2008 году, так и оставлю
                $p_num_end = $pages - 1;
                $in_end = $pages - $step - 1;

                //Генерируем шаблон страниц
                $tmplPages = '';
                $j = 0;
                while($j < $pages) {
                    //А ХЗ что тут
                    if (($j < $p_start && $j > 0 && $j < $in_end) || ($j > $p_end && $j < $p_num_end)) {
                        $j++;
                        continue;
                    }

                    $end = ($j == $p_num_end && $nowPage < ($in_end+2)) ? " .. " : null;
                    $begin = ($j == 0 && $nowPage >= $step) ? ".. " : null;

                    $p=$j+1;
                    $p = strlen($p) < 2 ? "0".$p : $p;

                    //Пусть к странице "/path/page/?page=22"
                    $pagePATH = $uri.$pfx.$nameGET.'='.$j;

                    if ($nowPage == $j) {
                        $tmplPages.= $end;
                        $tmplPages .= str_replace(
                            array('{path}', '{page}'),
                            array($pagePATH, $p),
                            $pageActiveTMPL
                        );
                    }
                    else {
                        $tmplPages .= $end;
                        $tmplPages .= str_replace(
                            array('{path}', '{page}'),
                            array($pagePATH, $p),
                            $pageTMPL
                        );
                        $tmplPages .= $begin;
                    }
                    $j++;
                } //while($j < $pages)

                $return->pages = $prevLINK.$tmplPages.$nextLINK;

            }//if ($pages > $maxPages)
            else {
                if ($nowPage > $pages) $nowPage = $pages - 1; //Если страница больше чем всех страниц, то назначаем наибольшую
                elseif ($nowPage < 0) $nowPage = 1; //Если страница меньше чем первая страница, то назначаем первую
                $tmplPages = '';
                for ($a=0; $a < $pages; $a++) {
                    //Пусть к странице "/path/page/?page=22"
                    $pagePATH = $uri.$pfx.$nameGET.'='.$a;

                    $p=$a+1; //Отображаемая страница
                    $p = strlen($p) < 2 ? "0".$p : $p;

                    //Если текущая страница
                    if ($nowPage == $a)  {
                        $tmplPages .= str_replace(
                            array('{path}', '{page}'),
                            array($pagePATH, $p),
                            $pageActiveTMPL
                        );
                    }
                    else {
                        $tmplPages .= str_replace(
                            array('{path}', '{page}'),
                            array($pagePATH, $p),
                            $pageTMPL
                        );
                    }

                }//for

                $return->pages = $prevLINK.$tmplPages.$nextLINK;

            } //else <- if ($pages > $maxPages)

        }//if ($total > limit)

        return $return;
    }

    /**
     * @param bool $total
     * @param bool $limit
     * @param bool $now_page
     * @param array $attr
     * @return stdClass | bool
     */
    final static public function get_pages_struct($total=false, $limit=false, $now_page=false, $attr=array()) {
        $now_page = $now_page > 0 ? $now_page : 1;

        $page = new stdClass();

        $isPage = $now_page - 1;

        //Удаляем из адресной строки ?page=<..>
        $from=array("#(/)?:?(highlight|page):[0-9]+/?#i");
        $pfx = stristr($_SERVER["REQUEST_URI"], "/:page") ? "/:" : (stristr($_SERVER["REQUEST_URI"], "/:") ? '/' : ':');
        $link_page=preg_replace($from, null, $_SERVER["REQUEST_URI"]);
        $page->list = '';

        //Если лимитов меньше 1 то лимит = 1
        if ($limit < 1 ) $limit=1;

        //Всего страниц (округл. в большую сторону)
        $pages=ceil($total/$limit);

        //Если всего страниц одна, тогда просто возвращаем false
        if ($pages < 2) return false;

        if ($isPage && $isPage > 0 && $now_page <= $pages) { //Если мы на странице
            //Вычисляем с какой строки и сколько выбрать из БД
            $page->limit=($limit*$isPage).", $limit";
        }
        elseif($now_page > $pages) {
            $page->limit=($limit*($pages-1)).", $limit";
            $now_page = $isPage = $pages;
        }
        else { //Иначе мы на первой странице
            $page->limit=$limit;
            $now_page = 1;
        }

        //Стрелки или слова "Вперед", "Назад
        if ($attr["btnPrev"]) $btnPrev = $attr["btnPrev"];
        else $btnPrev = '&lt;&lt;';

        if ($attr["btnNext"]) $btnNext = $attr["btnNext"];
        else $btnNext = '&gt;&gt;';

        //Вычисляем страницу предыдущую. Если она будет меньше 0, тогда переместит на последнюю
        if (($pagePrev=$now_page-1) < 1) $pagePrev = $pages;

        //Вычисляем страницу следующую. Если она будет > всего, тогда переместит на первую
        if (($pageNext=$now_page+1) > $pages) $pageNext = 1;

        //TODO: Надо убрать как то эту хуйню или сделать по другому
        //Блок яваскрипта
        $page->script = '<script>if (!isFunction("e2_func_get_pages")) { function e2_func_get_pages($link_page, $obj) {if (event.keyCode == 13) { if ($obj.value && $obj.value > 0) document.location.href=$link_page + $obj.value; return false;  } else return true;}}</script>';

        //Блок страниц
        $page->pages = '<div class="e2_func_get_pages">';
        $page->pages .= '<a href="'.$link_page.$pfx.'page:'.$pagePrev.'">'.$btnPrev.'</a> ';
        $page->pages .= '<input type="text" value="'.($now_page).'" OnKeyDown="return e2_func_get_pages(\''.$link_page.$pfx.'page:\', this);" />';
        $page->pages .= ' ~ '.$pages;
        $page->pages .= ' <a href="'.$link_page.$pfx.'page:'.$pageNext.'">'.$btnNext.'</a>';
        $page->pages .= '</div>';

        return $page;
    }

    final static public function nl2p($txt) {
        return str_replace("\n", '<p />', $txt);
    }

    final static public function nl2br_r($txt, $p=false, $one=false) {
        $from[] = "#(<br \/>|<\/li>|<\/div>|<\/ul>|<(\/)?table(.+)?>|<(\/)?tr(.+)?>|<(\/)?td(.+)?>|<(\/)?th(.+)?>|<(\/)?tbody(.+)?>|<ul(.+)?>|<\/p>|<(\/)?h(1|2|3|4|5|6)>)(\r\n|\n)+#i";
        $to[] = "$1";

        $txt=preg_replace($from, $to, $txt);

        $txt=nl2br($txt);
        $one = $one ? "1" : "2";
        if ($p) $txt=preg_replace('#((<br>|<br\s\/>)[\r\n]*){'.$one.',}#i', "<p />", $txt);
        else $txt=preg_replace('#((<br\s\/>)[\r\n])*){2,}#is', "<br />", $txt);

        return $txt;
    }

    final static public function br2nl($txt=null) {
        $from = array('<br>', '<br />');
        return str_ireplace($from, "\n", $txt);
    }

    final static public function days($data_from=false, $data_to=false) {
        if (!$data_from) $data_from = date("Y-m-d H:i:s");
        if (!$data_to) $data_to = date("Y-m-d H:i:s");

        $time_from=strtotime($data_from);
        $time_to=strtotime($data_to);

        $razn = ($time_to-$time_from);

        if ($razn) {
            $days = ceil($razn/(60*60*24));
        }

        return $days;

    }

    /**
     * Форматируем дату исходя из шаблона
     *
     * echo func::datas('2018-12-15 17:01:23', 'd.m.Y H:i'); //15.12.2018 17:01
     *
     * @param bool $data
     * @param string $format
     * @return bool|false|null|string
     */
    final static public function data($data=false, $format="d.m.Y H:i:s") {
        $data = $data  ? $data : null;
        if ($data == "0000-00-00 00:00:00" || !$data) return null;


        $time = strtotime($data);
        if ($time) {
            $data=date($format, $time);
            return $data;
        }
        return null;
    }

    /**
     * Обрезает $len символов с конца строки.
     *
     * $ins = '';
     * $and = '';
     * for($a=0;$a<5;$a++) {
     *      $ins .= 'var=val,';
     *      $and .= 'var=val && ';
     * }
     * $ins = func::sub_var($ins); //Уберет последнюю запятую
     * $ins = func::sub_var($ins, 5); //Уберет ' && ' с конца
     *
     * @param bool $var
     * @param int $len
     * @return string
     */
    final static public function sub_var($var=false, $len=1) {
        return mb_substr($var, 0, mb_strlen($var, "UTF-8")-$len, "UTF-8");
    }

    /**
     * Получает из массива строку для импорта в БД
     *
     * $post = array(
     *      'title' => 'Имя',
     *      'weight' => 300,
     *      'addr' => '123321, Singapore, Agarty office street',
     *      'age' => null
     * );
     *
     * // `title`='Имя', `weight`='300', `addr`='123321, Singapore, Agarty office street', 'age'=NULL
     * $ins = func:get_var($post);
     *
     * Если параметр передать массивом
     * $post = array(
     *      'name' => 'AAA-A.ru',
     *      'prj' => array(
     *          'agarty.net',
     *          'return.su',
     *          'apache22.com'
     *      )
     * );
     *
     * //`name`='AAA-A.ru', `prj`='agarty.net; return.su; apache22.com;'
     * $ins = func:get_var($post);
     *
     * Разделитель '; ' можно задавать свой (####---)
     * $ins = func:get_var($post, false, '####---');
     *
     * Если $fields = true, тогда
     *  $post = array(
     *      'title' => 'Имя',
     *      'weight' => 300,
     *      'addr' => '123321, Singapore, Agarty office street',
     *      'age' => null
     *  );
     *  $ins = func:get_var($post, true);
     * вернет:
     *  $ins['var']; //(`title`, `weight`, `addr`, `age`)
     *  $ins['val']; //('Имя', '300', '123321, Singapore, Agarty office street', NULL)
     *
     * Если $nohencode = true, тогда значения не будут конвертироваться в безопасную запись для БД
     *
     * @param bool $post
     * @param bool $fields
     * @param string $delim
     * @param bool $nohencode
     * @return array|string
     */
    final static public function get_var($post=false, $fields=false, $delim = "; ", $nohencode=false) {
        $tvar=$tval=$var = "";

        foreach ($post as $key=>$val) {
            if (is_array($val)) {
                $arvals = "";
                foreach	($val as $arval) {
                    $arvals .= trim($arval).$delim;
                }
                $val = $arvals;
            }

            if ($fields) {
                if (!$nohencode) $val=self::hencode(trim($val), true);
                $tvar .= "`$key`,";
                $tval .= ($val || is_numeric($val) ? "'$val',"  : "NULL,");
            } else {
                if (!$nohencode) $val=self::hencode(trim($val), true);
                $key=trim($key);
                $var .= "`$key`=".($val || is_numeric($val) ? "'$val',"  : "NULL,");
            }
        }

        if ($fields) {
            $var = array();
            $var["var"]="(".self::sub_var($tvar).")";
            $var["val"]="(".self::sub_var($tval).")";

        } else {
            $var = self::sub_var($var);
        }

        return $var;
    }

    final static public function firstbr($txt=false) {
        $txt=preg_split("#(\n|(<|&lt;)br /(&gt;|>)|(<|&lt;)p /(&gt;|>))#", $txt, 2);
        return strip_tags(self::hdecode($txt[0], true));
    }

    /**
     * Изменение размера изображения
     *  $attr = array(
     *      'from' => '/path/to/img_src.jpg', //Файл источник
     *      'to' => '/path/tor/img_dst.jpg', //Файл приемник
     *      'width' => 200, //Ширина,
     *      'height' => 150, //Высота,
     *      'q' = 90, //Качество JPEG
     *      'ratio' => true, //Соблюдать соотношение сторон
     * );
     * @param array $attr
     * @return bool
     */
    final static public function resizeIMG(array $attr) {

        $size_img = getimagesize($attr['from']);

        $attr['width'] = $attr['width'] ? $attr['width'] : $size_img[0];
        $attr['height'] = $attr['height'] ? $attr['height'] : $size_img[1];

        $ratio=$attr['width']/$attr['height'];

        if (!$size_img) return false;
        if (($size_img[0] < $attr['width']) && ($size_img[1] < $attr['height'])) return true;

        $src_ratio=$size_img[0]/$size_img[1];

        if ($attr['ratio']) {
            if ($ratio < $src_ratio) $attr['height'] = ceil($attr['width']/$src_ratio);
            else $attr['width'] = ceil($attr['height']*$src_ratio);
        }

        $dest_img=imagecreatetruecolor($attr['width'],$attr['height']);

        $src_img = null;

        if ($size_img[2] == 2) $src_img=imagecreatefromjpeg($attr['from']);
        elseif ($size_img[2] == 1)  $src_img=imagecreatefromgif($attr['from']);
        elseif ($size_img[2] == 3)  {
            $src_img=imagecreatefrompng($attr['from']);
            imagealphablending($src_img, false);
            imagesavealpha($src_img, true);

            imagealphablending($dest_img, false);
            imagesavealpha($dest_img, true);
        }

        if (!imagecopyresampled($dest_img, $src_img, 0, 0, 0, 0, $attr['width'], $attr['height'],
            $size_img[0], $size_img[1])) return false;

        if ($size_img[2] == 2)  imagejpeg($dest_img,$attr['to'],$attr['q']);
        elseif ($size_img[2] == 1)  imagegif($dest_img,$attr['to']);
        elseif ($size_img[2] == 3)  imagepng($dest_img,$attr['to']);

        imagedestroy($dest_img);
        imagedestroy($src_img);

        return is_file($attr['to']);
    }

    /**
     * Рекурсивное удаление каталога
     *
     * @param $directory
     * @return bool
     */
    final static public function rmdir_r($directory) {
        $dir = opendir($directory);
        while(($file = readdir($dir))) {
            if ( is_file ($directory."/".$file)) {
                unlink ($directory."/".$file);
            }
            else if ( is_dir ($directory."/".$file) &&
                ($file !== ".") && ($file !== "..")) {
                self::rmdir_r($directory."/".$file);
            }
        }
        closedir ($dir);
        rmdir ($directory);
        return true;
    }

    /**
     * Функция транслита
     * TODO: подумать под разные языки
     *
     * @param $text
     * @param bool $rus
     * @param bool $c
     * @param bool $y
     * @return mixed
     */
    final static public function translit($text, $rus=false, $c=false, $y=false) {

        $russian=array("А","Б","В","Г","Д","Е","Ё","Ж","З","И","Й","К","Л","М","Н","О","П","Р","С","Т","У","Ф","Х","Ц","Ч","Ш","Щ","Ь","Ы","Ъ","Э","Ю","Я",
            "а","б","в","г","д","е","ё","ж","з","и","й","к","л","м","н","о","п","р","с","т","у","ф","х","ц","ч","ш","щ","ь","ы","ъ","э","ю","я");


        $lat=array("A","B","V","G","D","E","E","Z","Z","I","I","K","L","M","N","O","P","R","S","T","U","F","H","C","CH",
            "SH","SH", "","Y","","E","JU","YA",
            "a","b","v","g","d","e","e","z","z","i","i","k","l","m","n","o","p","r","s","t","u","f","h","c","ch",
            "sh","sh","","y","","e","ju","ya"
        );


        if ($rus) {
            $text=str_replace($lat, $russian, $text);
            if ($c) $text=str_replace(array("Ц", "ц"), array("С", "с"), $text);
            if ($y) $text=str_replace(array("Ы", "ы"), array("И", "и"), $text);
        }
        else {
            $text=str_replace($russian, $lat, $text);
        }

        return $text;

    }

    final static public function translitClear($txt) {
        $txt = self::translit($txt);
        $txt = preg_replace(
            array(
                '#[^0-9_a-zA-Z\-\._]#',
                '#_{2,}#'
            ),
            '_',
            $txt
        );

        return $txt;
    }

    final static public function quote_encode($Tusik=false) {
        $Tuzik = array("'" => "&#039;", '"' => "&quot;");
        return strtr($Tusik, $Tuzik);
    }

    final static public function quote_decode($text=false) {
        $quotes = array("&#039;" => "'", "&quot;" => '"');
        return strtr($text, $quotes);
    }

    final static public function hdecode($body=false, $quotes=true, $slashes = true) {
        $body = $quotes ? htmlspecialchars_decode($body, ENT_QUOTES) : htmlspecialchars_decode($body);
        if ($slashes) $body = self::slash_decode($body);
        return $body;
    }

    final static public function hencode($body=false, $quotes=true, $slashes = true) {
        $body = $quotes ? htmlspecialchars($body, ENT_QUOTES, "UTF-8") : htmlspecialchars($body, false, "UTF-8");
        if ($slashes) $body = self::slash_encode($body);
        return $body;
    }

    final static public function slash_encode($body) {
        return strtr($body, array('\\' => '&#92;'));
    }

    final static public function slash_decode($body) {
        return strtr($body, array('&#92;' => '\\'));
    }

    final static public function lowline_encode($body) {
        return strtr($body, array('_' => '&#95;'));
    }

    final static public function lowline_decode($body) {
        return strtr($body, array('&#95;' => '_'));
    }

    final static public function get_youtube($link=false, $w=400, $h=300, $toreturn=false) {
        if (!$link) return false;
        preg_match("#\?v=(.+)#", $link, $m);

        $return["link"] = $link;
        $return["id"]=$m[1];
        $return["img0"]='http://img.youtube.com/vi/'.$m[1].'/0.jpg';
        $return["img1"]='http://img.youtube.com/vi/'.$m[1].'/1.jpg';
        $return["img2"]='http://img.youtube.com/vi/'.$m[1].'/2.jpg';
        $return["img3"]='http://img.youtube.com/vi/'.$m[1].'/3.jpg';

        $return["iframe"]='<iframe width="'.$w.'" height="'.$h.'" src="//www.youtube.com/embed/'.$m[1].'" frameborder="0" allowfullscreen></iframe>';
        $return["nocookie"]='<iframe width="'.$w.'" height="'.$h.'" src="//www.youtube-nocookie.com/embed/'.$m[1].'" frameborder="0" allowfullscreen></iframe>';

        $return["old"]='<object width="'.$w.'" height="'.$h.'"><param name="movie" value="//www.youtube-nocookie.com/v/'.$m[1].'?hl=ru_RU&amp;version=3"></param><param name="allowFullScreen" value="true"></param><param name="allowscriptaccess" value="always"></param><embed src="//www.youtube-nocookie.com/v/'.$m[1].'?hl=ru_RU&amp;version=3" type="application/x-shockwave-flash" width="420" height="315" allowscriptaccess="always" allowfullscreen="true"></embed></object>';
        $return["oldnocookie"]='<object width="'.$w.'" height="'.$h.'"><param name="movie" value="//www.youtube-nocookie.com/v/'.$m[1].'?hl=ru_RU&amp;version=3"></param><param name="allowFullScreen" value="true"></param><param name="allowscriptaccess" value="always"></param><embed src="//www.youtube-nocookie.com/v/'.$m[1].'?hl=ru_RU&amp;version=3" type="application/x-shockwave-flash" width="'.$w.'" height="'.$h.'" allowscriptaccess="always" allowfullscreen="true"></embed></object>';

        if ($toreturn) $return = $return[$toreturn];

        return $return;

    }

    final static public function mkdir_r($path=false) {
        $arr = explode("/", $path);
        $path_s = "";
        foreach ($arr as $val) {
            $val = trim($val);
            if (!$val) continue;
            $path_s .= "/".$val;
            if (!is_dir($path_s)) {
                mkdir($path_s);
                chmod($path_s, 0777);
            }
        }
    }

    final static public function dirsize($directory) {
        $size=0;
        $dir = opendir($directory);
        while(($file = readdir($dir))) {
            if ( is_file ($directory."/".$file)) {
                $size = $size + filesize ($directory."/".$file);
            }
            else if ( is_dir ($directory."/".$file) &&
                ($file !== ".") && ($file !== "..")) {
                $size = $size + self::dirsize($directory."/".$file);
            }
        }
        closedir ($dir);

        return $size;
    }

    final static public function ext($path=false, $dot=false) {
        $pathinfo = pathinfo($path);
        return $dot.$pathinfo['extension'];
    }

    final static public function numeric($txt=false, $attr=array()) {
        $preg = $attr['float'] ? '#[^0-9\.]#u' : '#[^0-9]#';
        $int = preg_replace($preg, null, $txt);
        return  $int ? $int : null;
    }

    final static public function psplit($txt=false, $split="\n", $mod="iu") {
        $preg = "#[".preg_quote($split)."]#u".$mod;
        return preg_split($preg, $txt, false, PREG_SPLIT_NO_EMPTY);
    }

    final static public function pclear($txt='', $preg="[^a-zA-Z0-9]", $mod="i") {
        $preg = "#".$preg."#u".$mod;
        return preg_replace($preg, null, $txt);
    }

    final static public function encode($string=false, $to="UTF-8", $from="cp1251") {
        return @mb_convert_encoding($string, $to, $from);
    }

    /**
     * Вернет сальт из указанного кол-ва исмволов (не менее 6)
     *
     *  $salt = func::salt(); //Вернет, что-то вроде: @G#?s|8w#^K#&3
     *
     * @param bool $num
     * @return mixed|string
     */
    final static public function salt($num=false) {
        $salt="";
        $num = $num >= 6 ? $num : 20;
        for($a=1; $a <= $num; $a++) {
            $fRand = rand(1, 3);

            $rand = $a%2 ? rand(65, 90) : rand(97, 122);
            $salt .= chr($rand);
        }

        if (mb_strlen($salt, "UTF-8") < $num) $salt = self::salt($num);

        return $salt;
    }

    final static public function get_headers() {
        foreach($_SERVER as $key=>$val) {
            if (preg_match('#^HTTP_#', $key)) {
                $key = str_replace(" ", "-", ucwords(strtolower(str_replace("_", " ", substr($key, 5)))));
                $headers[$key]=$val;
            } elseif ($key == "CONTENT_TYPE") {
                $headers["Content-Type"] = $val;
            } elseif ($key == "CONTENT_LENGTH") {
                $headers["Content-Length"] = $val;
            }
        }

        return $headers;
    }

    final static public function get_json($arr=array()) {

        //Обход массива с языком
        $jsone="";
        foreach($arr as $name=>$value) {
            if ($value === null) continue;

            //Добавляем экранирующий символ на знак ' (одиночная кавычка)
            $value = str_replace('"', '\"', $value);
            $name  = str_replace('"', '\"', $name);

            //Добавляем в строку соответвие
            $jsone .= '"'.$name.'": "'.$value.'",';
        }
        //Убираем последний запятую
        $jsone = func::sub_var($jsone, 1);

        return $jsone ? "{".$jsone."}" : null;

    }

    public final static function chk_field_format($txt="", $type="") {

        switch($type) {
            case "mail":
                    return preg_match('#^[0-9a-zA-Z_\-\.а-яА-ЯёЁ]+@[0-9a-zA-Z_\-\.а-яА-ЯёЁ]+\.[a-zA-Z0-9а-яА-ЯёЕ]{2,6}$#', $txt);
                break;

            case "numeric":
                    return preg_match('#^[0-9]+$#', $txt);
                break;

            default:
                return false;
        }

    }

    /**
     * @param string $arr
     * @param bool $obj
     * @return array|null
     */
    public final static function clear_arr(array $arr, $obj=true) {
        if (!is_array($arr)) return null;
        //Обходим массив
        foreach($arr as $key=>$val) {
            //Если значение является массивом, рекурсивно вызывает текущий метод
            if (is_array($val)) {
                if ($obj && $key && !is_numeric($key)) $return->$key = self::clear_arr($val, $obj);
                else $return[$key] = self::clear_arr($val, $obj);
            } else { //Иначе обрабатываем
                //Если указано возвратить объект, иначе массив
                if ($obj && $key && !is_numeric($key)) $return->$key = trim(self::hencode($val));
                else $return[$key] = trim(self::hencode($val));
            }

        }

        return $return;
    }

    /**
     * Очищает в массиве пробелы перед и после слов
     *
     * @param array $arr
     * @param bool $noEmpty
     * @return null|string|string[]
     */
    public final static function trimArray(array $arr, $noEmpty=false) {
        foreach ($arr as $key=>$val) {
            $arr[$key] = trim($val);

            if ($noEmpty && !$arr[$key]) {
                unset($arr[$key]);
            }
        }

        return $arr;
    }

    public final static function ClearName($name="", $pattern='') {
        $pattern = $pattern ? $pattern : '#[^a-zA-Z_0-9]#';
        return preg_replace($pattern, null, $name);
    }

    private static $is_noEmpty=false;
    public final static function is_empty($txt) {
        if (!self::$is_noEmpty) {
            if (is_array($txt)) {
                foreach($txt as $val) {
                    if (!self::$is_noEmpty) self::is_empty($val);
                }
            }
            elseif (trim($txt)) self::$is_noEmpty=true;
        }

        return self::$is_noEmpty ? false : true;
    }

    public final static function print_r_echo($array="", $nopre=false) {
        $return = print_r($array, true);

        if (!$nopre) {
            $return = '<pre>'.$return.'</pre>';
        }

        return $return;
    }

    final public static function print_r_comment($array=null) {
        $return = '<!--' . print_r($array, true) . '-->';
        return $return;
    }

    /**
     * Публичный метод, проверяющий пароли по базе $path.
     * Если пароль будет найден - вернет true;
     *
     * if (func::known_passwords(123987, '/path/to/db.txt')) $ERROR = 'Слишком слабый пароль';
     *
     * Где /path/to/db.txt, обычный текстовый файл, содержащий пароли, каждый на новой строке.
     *
     * @param string $pass
     * @param string $path
     * @return bool
     */
    private static $known_passwords=array();
    public final static function known_passwords($pass='', $path='') {
        $pass = mb_strtolower($pass, 'UTF-8');
        if (!$pass) return true;

        if (!self::$known_passwords) {
            if (@is_file($path)) self::$known_passwords = @file($path, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        }

        foreach (self::$known_passwords as $password) {
            if ($pass == $password) return true;
        }

        return false;
    }

    /**
     * Принимает текст, и если он = null - возвращает символ неразрывного
     * пробела или то, что передается параметром $nbsp.
     *
     * @param string $text
     * @param null $nbsp
     * @return string
     */
    public final static function nullspace($text='', $nbsp=null) {
        $text = trim($text);
        return $text ? $text : (trim($nbsp) ? $nbsp : '&nbsp;');
    }

    /**
     * Шифрует данные.
     *
     * Принимает данные и ключ.
     * @param string $data
     * @param string $key
     * @return string
     */
    public final static function encrypt($data='', $key='') {
        return openssl_encrypt($data, 'AES-256-CBC', $key);
    }

    /**
     * Дешифрует данные.
     *
     * Принимает данные и ключ.
     * @param $action
     * @param $string
     * @return string
     */
    public final static function encrypt_decrypt($action='', $string='') {
        $output = false;
        $encrypt_method = "AES-256-CBC";
        $secret_key = 'This is my secret key';
        $secret_iv = 'This is my secret iv';
        // hash
        $key = hash('sha256', $secret_key);

        // iv - encrypt method AES-256-CBC expects 16 bytes - else you will get a warning
        $iv = substr(hash('sha256', $secret_iv), 0, 16);
        if ( $action == 'encrypt' ) {
            $output = openssl_encrypt($string, $encrypt_method, $key, 0, $iv);
            $output = base64_encode($output);
        } else if( $action == 'decrypt' ) {
            $output = openssl_decrypt(base64_decode($string), $encrypt_method, $key, 0, $iv);
        }
        return $output;
    }


    public final static function echoexit($data='', $args=array()) {
        if ($args['pre']) echo '<pre>';

        if (is_array($data) OR is_object($data)) {
            if ($args['hencode']) echo self::hencode(print_r($data, true)), 1;
            else print_r($data);
        } else {
            if ($args['hencode']) $data = self::hencode($data);
            echo $data;
        }
        if ($args['pre']) echo '</pre>';
        if (!$args['noexit']) exit;
    }

    //Массив аттрибутов
    public final static function attrs($return='') {

        switch ($return) {
            case 'numbers': $return = array(0, 5, 7); break;
            default:
                $return = array(
                    0 => lang::txt('attr_noacces'),
                    5 => lang::txt('attr_read'),
                    7 => lang::txt('attr_read_write')
                );
        }

        return $return;

    }

    /**
     * Проверка пароля пользователя на простоту и совпадение введенных паролей
     *
     * @param array $attr
     * @return bool
     */
    public final static function check_pass(array $attr) {
        //Длина пароля
        $pass_len = $attr['pass_len'] > 0 ? $attr['pass_len'] : 6;

        //Если поле пароля пустое, или пароли не совпадают
        if ((!empty($attr['pass']) OR !empty($attr['pass_conf'])) && ($attr['pass'] !== $attr['pass_conf'])) {
            errors::set(lang::txt("chkerror_pass"));
            $noValid = true;
        }
        //Проверка пароля
        else {

            //Если указано проверять пароль по базе (/_elfium/lib/passwords.txt)
            if ($attr['checkPassFromDB']) {
                //Поиск в базе популярных паролей
                if (func::known_passwords($attr['pass'], _EROOT.'/lib/passwords.txt')) {
                    errors::set(lang::txt("chkerror_pass_nostrong"));
                    $noValid = true;
                }
            }

            //Длина пароля
            if ($pass_len <= 0 OR mb_strlen($attr['pass'], 'UTF-8') < $pass_len) {
                errors::set(str_replace("%len%", $pass_len, lang::txt("chkerror_pass_len")));
                $noValid = true;
            } else {
                //На всякий пожарный
                $pass = $attr['pass'];

                //Пароль не должен состоять только из одного символа по всей длине
                if (preg_match('#^'.$pass{0}.'+$#i', $pass)) {
                    errors::set(lang::txt("chkerror_pass_nostrong"));
                    $noValid = true;
                }
            }

        }

        return $noValid ? false : true;
    }

    /**
     * Метод возвращающий преобразованное имя пользователя
     *
     * @param $user
     * @param $format
     * @return mixed
     */
    public final static function userName($user, $format="%s% %f..%%t..%") {
        /*
         $format =
              %s% - second_name АгАртИ
              %f% - first_name  МРиТьЮ
              %t% - third_name  СарВеШу
              %s.%, %f.%, %t.% - Сокращенные
              %s% %f.%%t.% - АгАртИ М.С.
       */

        //Выбираем первые буквы
        $f = preg_split('/(?=(.{1})*$)/u', $user->first_name);
        $s = preg_split('/(?=(.{1})*$)/u', $user->second_name);
        $t = preg_split('/(?=(.{1})*$)/u', $user->third_name);

        $replace = array(
            "%s%" => $user->second_name ? $user->second_name : null,
            "%f%" => $user->first_name ? $user->first_name : null,
            "%t%" => $user->third_name ? $user->third_name : null,
            "%s.%" => $s[1] ? $s[1] : null,
            "%f.%" => $f[1] ? $f[1] : null,
            "%t.%" => $t[1] ? $t[1] : null,
            "%s..%" => $s[1] ? $s[1].'.' : null,
            "%f..%" => $f[1] ? $f[1].'.' : null,
            "%t..%" => $t[1] ? $t[1].'.' : null
        );

        $from = array_keys($replace);
        $to = array_values($replace);

        return str_replace($from, $to, $format);
    }

    /**
     * Возвращает текущую строку
     * Содержит массив с параметрами
     *
     * $URI = func::thisPage(array(
     *   'urlencode' //Перекодировать с помощью функции urlencode()
     * ));
     *
     * @param $attr
     * @return string
     */
    public static function thisPage($attr) {
        $return = $_SERVER['REQUEST_URI'];

        if (in_array('urlencode', $attr)) {
            $return = urlencode($return);
        }

        return $return;
    }

    public static function clearField($field) {
        return preg_replace('[^0-9a-zA-Z_]', null, $field);
    }

    /**
     * PANDORA FUNC
     */
    public static function number_money_average($num, $total, $format) {
        $return = null;

        switch ($format) {
            case 'ceil':
                $return = number_format(ceil($num/$total), null, false, ' ');
                break;

            case 'round':
                $return = number_format(round($num/$total, 2), 2, ',', ' ');
                break;

            case 'summ':
                $return = number_format(round($num, 2), 2, ',', ' ');
                break;
        }
        return $return;
    }

    /**
     * Вывод мессажа или сохранение в лог
     *
     * @param $txt
     */
    public static function echoMsg($txt) {
        if ($txt) echo date('Y-m-d H:i:s ').$txt."\n";
        else echo "\n";
    }

    /**
     * Преобразование и замена точки на запятые
     *
     * @param $num
     * @param $dec
     * @return mixed
     */
    public static function myRound($num, $dec) {
        return str_replace('.', ',', round($num, $dec));
    }

    /**
     * Разница дат
     *
     * @param $time
     * @return string
     */
    public static function hms($time) {
        if (!$time) $time =0;
        $HMS = new DateTime('@'.ceil($time));
        return $HMS->format('H:i:s');
    }

    public static function dataOffset($data, $format, $offset) {
        if (!$data) return null;
        if ($data == "0000-00-00 00:00:00") return null;
        if ($data !== 'now') {
            $data = strtotime($data);
        }

        $date = new DateTime();
        $date->setTimestamp($data);
        $date->setTimezone(new DateTimeZone(self::offset($offset, '%+02d%02d')));
        return $date->format($format);
    }

    public static function offset($mins, $pattrn) {
        $sgn = ($mins < 0 ? -1 : 1);
        $mins = abs($mins);
        $hrs = floor($mins / 60);
        $mins -= $hrs * 60;
        return sprintf($pattrn ? $pattrn : '%+d:%02d', $hrs*$sgn, $mins);
    }

    /**
     *
     *
     * @param $val1
     * @param $val2
     * @return null|string
     */
    public static function selected($val1, $val2) {
        if (!trim($val1) || !trim($val2)) return null;
        return $val1 == $val2 ? 'selected' : null;
    }

    /**
     * Формирует тэг select
     *
     * $options = array('1', '2', 'three', 'option');
     * или
     * $options = array('1020' => 'Apple', '1030' => 'Juice', '1040' => 'Green', 'value' => 'option');
     *
     * echo func::select([
            'name' => "field[name]", //Имя поля
            'id' => 'fruits', //ID поля
            'body' => 'value', //Что выбрать заранее, или выбранный ответ
            'style' => 'color: red;', //Стиль поля
            'attr' => 'OnClick="alert(1)" rel="alsoRel"', //Дополнительные параметры
            'options' => $options,
            'noEmpty' => true, //Без пустого значения
            'emptyTitle' => null, //Если noEmpty = true, добавляет заголовок к пустому значению
            'onlyOptions' => false, //Если = true, тогда массив состоит только из значений option, иначе value => option
        ]);
     *
     * @param array $attr
     * @return null|string
     */
    public static function select(array $attr) {
        $return = null;

        //Если список имеется
        if (count($attr['options'])) {

            //Формируем тэг
            $return = '<select name="'.$attr['name'].'" id="'.$attr['id'].'" style="'.$attr['style'].'" '.$attr['attr'].'>';

            //Если не указано, что не нужно пустого значения - добавляем
            if (!$attr['noEmpty']) {
                $return .= '<option value="">'.$attr['emptyTitle'].'</option>';
            }

            //Если массив только из назнваний
            if ($attr['onlyOptions']) {

                //Формируем варианты выбора
                foreach ($attr['options'] as $option) {
                    $selected = func::selected($option, $attr['body']);
                    $return .= '<option value="'.$option.'" '.$selected.'>'.$option.'</option>';
                }

            }
            //Иначе по умолчанию названия и ключи
            else {

                //Формируем варианты выбора
                foreach ($attr['options'] as $value=>$option) {
                    $selected = func::selected($value, $attr['body']);
                    $return .= '<option value="'.$value.'" '.$selected.'>'.$option.'</option>';
                }

            }


            $return .= '</select>';
        }

        return $return;
    }

    public static function select_number($name, $numbers=5, $val=null, $noZero = false) {

        $return = '<select name="'.$name.'">';
        if (!$noZero) $return .= '<option value="0">0</option>';

        for($a = 1; $a <= $numbers; $a ++) {
            $selected = self::selected($a, $val);
            $return .= '<option value="'.$a.'" '.$selected.'>'.$a.'</option>';
        }

        $return .= '</select>';

        return $return;

    }

    public static function select_text($name, array $vals, $val = null, $noZero = false) {

        $return = '<select name="'.$name.'">';
        if (!$noZero) $return .= '<option value=""></option>';

        foreach ($vals as $txt) {
            $selected = self::selected($txt, $val);
            $return .= '<option value="'.$txt.'" '.$selected.'>'.$txt.'</option>';
        }

        $return .= '</select>';

        return $return;

    }

    public static function select_array_value($name, array $array, $val, $noZero=false, $addt = null) {

        $return = '<select name="'.$name.'" '.$addt.'>';
        if (!$noZero) $return .= '<option value=""></option>';

        foreach ($array as $value) {
            $selected = func::selected($value, trim($val));
            $return .= '<option value="'.trim($value).'" '.$selected.'>'.$value.'</option>';
        }

        $return .= '</select>';

        return $return;
    }

    public static function select_array_key($name, array $array, $val, $noZero=false, $addt=null) {

        $return = '<select name="'.$name.'" '.$addt.'>';
        if (!$noZero) $return .= '<option value=""></option>';

        foreach ($array as $key=>$value) {
            $selected = func::selected($key, $val);
            $return .= '<option value="'.$key.'" '.$selected.'>'.$value.'</option>';
        }

        $return .= '</select>';

        return $return;
    }

    /**
     * Метод работы с CSV файлом
     *
     * while($string = func::getCSV([
     *    'file' => '/path/to/file.csv',
     *    'delimeter' => ';', //По умолчанию ;
     *    'length' => '10000',  //По умолчанию 10000
     * ])) {
     *
     * }
     *
     */
    private static $getCSV = null;
    public static function getCSV(array $attr) {
        if (!self::$getCSV) {
            if (!is_file($attr['file'])) return false;
            self::$getCSV = fopen($attr['file'], "r");
        }

        if (!$return = fgetcsv(
            self::$getCSV,
            $attr['length'] ? $attr['length'] : 10000,
            $attr['delimeter'] ? $attr['delimeter'] : ';'
        )) {
            fclose(self::$getCSV);
        }

        return $return;
    }

    /**
     * Расстояние от точки до дочки исходя из координат
     * @param array $attr
     * @return float|int
     */
    public static function distance(array $attr) //$longitude1, $latitude1, $longitude2, $latitude2)
    {
        $latitude1 = $attr['lat1'] ? $attr['lat1'] : $attr['latitude1'];
        $latitude2 = $attr['lat2'] ? $attr['lat2'] : $attr['latitude2'];

        $longitude1 = $attr['lng1'] ? $attr['lng1'] : $attr['longitude1'];
        $longitude2 = $attr['lng2'] ? $attr['lng2'] : $attr['longitude2'];

        if (!$latitude1 || !$latitude2 || !$longitude1 || !$longitude2) return false;

        $earth_radius = 6372797; //средний радиус Земли в м

        $dLat = deg2rad($latitude2 - $latitude1);
        $dLon = deg2rad($longitude2 - $longitude1);

        $a = sin($dLat/2) * sin($dLat/2) + cos(deg2rad($latitude1)) * cos(deg2rad($latitude2)) * sin($dLon/2) * sin($dLon/2);
        $c = 2 * asin(sqrt($a));
        $d = $earth_radius * $c;

        return $d;
    }

    /**
     * Конвертируем файл в base64
     */
    /*
     * data:image/png;base64,
     * */
    public static function src2base64($path) {
        $return = null;
        //Проверяем, если он уже base64, тогда вернем сразу
        if (strpos($path, 'base64') !== FALSE) {
            $return = $path;
        }

        else {
            //Определяем тип файла (картинки)
            $ext = strtolower(self::ext($path));

            switch($ext) {
                case 'jpg':
                case 'jpeg':
                    $mime = 'image/jpeg';
                    break;

                case 'png':
                    $mime = 'image/png';
                    break;

                case 'gif':
                    $mime = 'image/gif';
                    break;

                default:
                    $mime = null;
            }

            //Если не поддерживаемый mime, вернем путь
            if (!$mime) {
                $return = $path;
            }
            else {
                //Проверяем пути
                $file = null;

                //Если ссылка
                if (strpos($path, 'http') !== FALSE) {
                    $file = $path;
                }
                //Если файл на сервере
                else if (is_file($_SERVER['DOCUMENT_ROOT'].$path)) {
                    $file = $_SERVER['DOCUMENT_ROOT'].$path;
                }

                //Если файл есть
                if ($file) {

                    //Строим base64
                    $return = 'data:';
                    $return .= $mime.';';
                    $return .= 'base64,';
                    $return .= base64_encode(file_get_contents($file));

                } else {
                    $return = $path;
                }
            }
        }

        return $return;
    }

    /**
     * Находим в тексте все src и конвертируем файлы
     */
    public static function src2base64_text($text) {
        return preg_replace_callback('#src=([\'"])([^"\']+)([\'"])#', function($m) {
            return 'src='.$m[1].func::src2base64($m[2]).$m[3];
        }, $text);
    }

    /**
     * Окончание год/лет
     *
     * @param null $num
     * @param string $lang
     * @return mixed|null
     */
    public static function num2word($num = null, $lang = 'ru') {
        if (!$num) return null;

        if ($lang == 'en') {
            $words = array('years old', 'years old', 'years old');
        } else {
            $words = array('год', 'года', 'лет');
        }

        $num = $num % 100;
        if ($num > 19) {
            $num = $num % 10;
        }
        switch ($num) {
            case 1: {
                return($words[0]);
            }
            case 2: case 3: case 4: {
            return($words[1]);
        }
            default: {
                return($words[2]);
            }
        }
    }

    public static function getFullYears($birthdayDate) {
        $datetime = new DateTime($birthdayDate);
        $interval = $datetime->diff(new DateTime(date("Y-m-d")));
        return $interval->format("%Y");
    }

    /**
     * Шифрование данных
     *
     * @param $data
     * @param $key
     * @return string
     */
    public static function encrypt_data($data, $key) {
        $ivlen = openssl_cipher_iv_length($cipher="AES-128-CBC");
        $iv = openssl_random_pseudo_bytes($ivlen);

        $ciphertext_raw = openssl_encrypt($data, $cipher, $key, $options=OPENSSL_RAW_DATA, $iv);
        $hmac = hash_hmac('sha256', $ciphertext_raw, $key, $as_binary=true);
        $ciphertext = base64_encode( $iv.$hmac.$ciphertext_raw );

        return $ciphertext;
    }

    /**
     * Шифрование данных
     *
     * @param $data
     * @param $key
     * @return string
     */
    public static function decrypt_data($data, $key) {
        $return = null;

        $data = base64_decode($data);

        $ivlen = openssl_cipher_iv_length($cipher="AES-128-CBC");
        $iv = substr($data, 0, $ivlen);

        $hmac = substr($data, $ivlen, $sha2len=32);
        $ciphertext_raw = substr($data, $ivlen+$sha2len);

        $original_plaintext = openssl_decrypt($ciphertext_raw, $cipher, $key, $options=OPENSSL_RAW_DATA, $iv);
        $calcmac = hash_hmac('sha256', $ciphertext_raw, $key, $as_binary=true);

        if (hash_equals($hmac, $calcmac)) {
            $return =  $original_plaintext;
        }

        return $return;
    }

    /**
     * Принимает номер месяца, возвращает его наименование
     * Если $aya = true, прибавляет окончание к месяцу
     *
     * @param $number
     * @param bool $aya
     * @return mixed|null
     */
    public function monthTitle($number, $aya=false) {
        if (!$monthTitle = $this->numeric($number)) return null;

        $months = array(
            '1' => 'Январь',
            '01' => 'Январь',
            '2' => 'Февраль',
            '02' => 'Февраль',
            '3' => 'Март',
            '03' => 'Март',
            '4' => 'Апрель',
            '04' => 'Апрель',
            '5' => 'Май',
            '05' => 'Май',
            '6' => 'Июнь',
            '06' => 'Июнь',
            '7' => 'Июль',
            '07' => 'Июль',
            '8' => 'Август',
            '08' => 'Август',
            '9' => 'Сентябрь',
            '09' => 'Сентябрь',
            '10' => 'Октябрь',
            '11' => 'Ноябрь',
            '12' => 'Декабрь'
        );

        $monthsAya = array(
            '1' => 'Января',
            '01' => 'Января',
            '2' => 'Февраля',
            '02' => 'Февраля',
            '3' => 'Марта',
            '03' => 'Марта',
            '4' => 'Апреля',
            '04' => 'Апреля',
            '5' => 'Мая',
            '05' => 'Мая',
            '6' => 'Июня',
            '06' => 'Июня',
            '7' => 'Июля',
            '07' => 'Июля',
            '8' => 'Августа',
            '08' => 'Августа',
            '9' => 'Сентября',
            '09' => 'Сентября',
            '10' => 'Октября',
            '11' => 'Ноября',
            '12' => 'Декабря'
        );

        return $aya ? $monthsAya[$number] : $months[$number];
    }
}