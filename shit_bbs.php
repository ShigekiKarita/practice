<?php

$log = new Log;
global $formUserName;
global $formUserComment;
global $formPostID;

if (isset($_POST['btn_submit'])) {
    $log->appendPost(htmlspecialchars($_POST['user_name']),htmlspecialchars($_POST['user_comment']));
}
else if (isset($_POST['btn_edit_submit'])) {
    $log->submitEditedPost($_POST['post_id'], htmlspecialchars($_POST['user_name']),htmlspecialchars($_POST['user_comment']));
}
else if (isset($_POST['btn_delete'])) {
    $log->deletePost(htmlspecialchars($_POST['delete_id']));
}
else if (isset($_POST['btn_edit'])) {
    $formPostID = htmlspecialchars($_POST['edit_id']);
    $editPost = $log->getPostOfID($formPostID);
    $formUserName = $editPost[1];
    $formUserComment = $editPost[2];
}

$log->showPosts();


class Log 
{
    public function __construct($logFilePath = 'log.txt', $countFilePath = 'count.txt') 
    {
        $this->logFile = $logFilePath;
        $this->countFile = $countFilePath;
        $this->createFile($logFilePath);
        $this->createFile($countFilePath);
    }
    public function appendPost($name, $comment) 
    {
        $time = time();
        $comment =  str_replace("\n","<br>",$comment);;        
        $id = $this->newID();
        $pieces = array($id, $name, $comment, $time);
        $line = implode($this->glue, $pieces) . "\n";
        if (!file_put_contents($this->logFile, $line, FILE_APPEND | LOCK_EX)) {
            die ('could not append the log');
        }
    }
    private function newID() 
    {
        if (file_exists($this->countFile)) {
            $currentID = file_get_contents($this->countFile);
        } else {
            $currentID = 0;
        }
        if ($currentID === FALSE) die ("could not get current id");
        $currentID = intval($currentID) + 1;
        if (!file_put_contents($this->countFile, $currentID, LOCK_EX)) {
            die ('could not append the id');
        }
        return $currentID;
    }
    public function showPosts()
    {
        $posts = file($this->logFile);
        if ($posts === FALSE) die ("could not open the log");
        foreach ($posts as $out) {
            $pieces = explode($this->glue, $out);
            $out = "<p>" . $pieces[0];
            $out = $out. ":\t " . $pieces[1];
            $out = $out. "\t at " . date('c', intval($pieces[3])) ."</p>";
            $out = $out."<p>".$pieces[2]."</p>";
            print $out;
        }
    }
    public function processPostInLog($postID, $func) {
        $posts = file($this->logFile);
        if ($posts === FALSE) die ("could not open the log");
        if (!unlink($this->logFile)) die ('could not clear the log');
        foreach ($posts as $p) {
            if (explode($this->glue, $p)[0] === $postID) {
                $func();
                continue;
            }
            if (!file_put_contents($this->logFile, $p, FILE_APPEND | LOCK_EX)) {
                    die ('could not rewrite the log');
            }
        }
    }
    public function deletePost($deleteID) 
    {
        $this->processPostInLog($deleteID, function(){});
    }
    public function getPostOfID($editID) 
    {
        $posts = file($this->logFile);
        if ($posts === FALSE) die ("could not open the log");
        foreach ($posts as $p) {
            $pieces = explode($this->glue, $p);
            if ($pieces[0] === $editID) {
                return $pieces;
            }
        }   
    }
    public function submitEditedPost($postID, $name, $comment) {
        $args = implode($this->glue, array($postID, $name, $comment, time()));
        $posts = file($this->logFile);
        if ($posts === FALSE) die ("could not open the log");
        if (!unlink($this->logFile)) die ('could not clear the log');
        foreach ($posts as $p) {
            if (explode($this->glue, $p)[0] === $postID) {
                if (!file_put_contents($this->logFile, $args . "\n", FILE_APPEND | LOCK_EX)) {
                    die ('could not rewrite the log');
                }
                continue;
            }
            if (!file_put_contents($this->logFile, $p, FILE_APPEND | LOCK_EX)) {
                    die ('could not rewrite the log');
            }
        }
    }
    public function getUserName() {
        return $this->formUserName;
    }
    public function getUserComment() {
        return $this->formUserComment;
    }
    private $logFile;
    private $countFile;
    private $glue = "<>";
    private function createFile($filePath) 
    {
        if(!file_exists($filePath)) {
            if(!touch($filePath)) die ("could not create new " . $filePath);
        }
        if(fileperms($filePath) !== 0666) {
            if(!chmod($filePath, 0666)) {
                die ("could not change the " . $filePath ." readable nor writable");
            }
        }
    }
    private function checkStr($str) {
        if (mb_strpos($str, $this->glue) === FALSE) {
            die ("contains forbidden word");
        }
    }
}
?>

<html>
<head>
    <meta charset="UTF-8" />
    <title>PHP TEST</title>
</head>
<body>

<form method="POST" action="<?php print($_SERVER['PHP_SELF']) ?>">
<input type="text" name="post_id" value="<?php print $formPostID;?>">
<input type="text" name="user_name" value="<?php print $formUserName;?>"><br>
<textarea name="user_comment" rows="8" cols="40" value = "comment"><?php print $formUserComment;?></textarea><br>
<input type="submit" name="btn_submit" value="Submit">
<input type="submit" name="btn_edit_submit" value="Edit Submit">

</form>

<form method="POST" action="<?php print($_SERVER['PHP_SELF']) ?>">
<input type="text" name="delete_id" value="enter id">
<input type="submit" name="btn_delete" value="Delete"><br />
<form>

<form method="POST" action="<?php print($_SERVER['PHP_SELF']) ?>">
<input type="text" name="edit_id" value="enter id">
<input type="submit" name="btn_edit" value="Edit"><br />
<form>

</body>
</html>
