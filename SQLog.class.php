<?php

class SQLog
{
    public function __construct($tableName = 'BBS') 
    {
        $this->tableName = $tableName;
        $this->connectDB();
        $query = "CREATE TABLE IF NOT EXISTS 
        $this->tableName(
            id int not null auto_increment primary key,
            name varchar(32) not null,
            comment text not null,
            time int unique,
            password varchar(32) not null
        )";
        $this->dbh->query($query);
        $this->disconnectDB();
    }
    private $dbh;
    private $tableName;
    private function connectDB() 
    {
        try {
            $this->dbh = new PDO('mysql:host=localhost;dbname= DB_NAME', 'USER_NAME', 'PASSWORD');
        } catch (PDOException $e) {
            var_dump($e->getMessage());
            echo ("could not connet to MySQL now");
            exit;
        }
    }
    private function disconnectDB() 
    {
        $this->dbh = NULL;
    }
    public function appendPost($name, $comment, $pw)
    {
        $time = time();
        $comment =  str_replace("\n","<br>",$comment);       
        $currentTime = time();

        $this->connectDB();
        $stmt = $this->dbh->prepare("insert into $this->tableName (name,comment, time, password) values (:name,:comment,:time,:password)");
        $stmt->execute(array(":name"=>$name, ":comment"=>$comment, ":time"=>$time, ":password"=>$pw));
        $this->disconnectDB();
    }
    public function showPosts()
    {
        $this->connectDB();
        $sql = "select * from $this->tableName order by id";
        $stmt = $this->dbh->query($sql);
        foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $post) {
            $out = "<p>" . $post['id'];
            $out = $out. ":\t " . $post['name'];
            $out = $out. "\t at " . date('c', intval($post['time'])) ."</p>";
            $out = $out."<p>".$post['comment']."</p>";
            print $out;
        }
        $this->disconnectDB();
    }
    public function deletePost($deleteID, $deletePW) 
    {
        $this->connectDB();
        $stmt = $this->dbh->prepare("delete from $this->tableName where id like :id and password like :pw");
        $stmt->execute(array(":id" => $deleteID, ":pw" => $deletePW)); 
        $this->disconnectDB();
    }
    public function getPostOfID($editID, $editPW) 
    {
        $this->connectDB();
        $sql = "select * from $this->tableName";
        $stmt = $this->dbh->query($sql);
        foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $post) {
            if ($post['id'] == $editID && $post['password'] == $editPW) {
                $this->disconnectDB();
                return $post;
            }
        }
        $this->disconnectDB();
        die ("invalid id or password.");
    }
    public function submitEditedPost($postID, $name, $comment) 
    {
        $this->connectDB();
        $comment =  str_replace("\n","<br>",$comment);
        $stmt = $this->dbh->prepare("update $this->tableName set name = :name, comment = :comment where id like :id");
        $stmt->execute(array("name" => $name, ":comment"=>$comment, ":id" => $postID)); 
        $this->disconnectDB();
    }
}
