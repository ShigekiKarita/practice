<?php

// http://qiita.com/tabo_purify/items/2575a58c54e43cd59630

// データベースへの接続
try {
    $dbh = new PDO('mysql:host=localhost;dbname=blog_app', 'dbuser', 'dbpassword');
}
catch (PDOException $e) {
    var_dump($e->getMessage());
    exit;
}

// 処理

// 追加 (prepared statement)
// $stmt = $dbh->prepare("insert into users (name,email,password) values (?,?,?)");
// $stmt->execute(array("n","e","p"));
$stmt = $dbh->prepare("insert into users (name,email,password) values (:name,:email,:password)");
$stmt->execute(array(":name"=>"n2",":email"=>"e2",":password"=>"p2"));
echo "done";

// 表示
$sql = "select * from users";
$stmt = $dbh->query($sql);
foreach ($stmt->fetchAll(PDO::FETCH_ASSOC) as $user) {
    var_dump($user['name']);
}
echo $dbh->query("select count(*) from users")->fetchColumn() . " records found";

// 変更
$stmt = $dbh->prepare("update users set email = :email where name like :name");
$stmt->execute(array(":email"=>"dummy", ":name" => "n%")); // nで始まるユーザのemailをdummyに

// 削除
$stmt = $dbh->prepare("delete from users where password = :password");
$stmt->execute(array(":password"=>"p10")); 

// 削除件数
$stmt->rowCount() . " records deleted";

// 切断
$dbh = null;
