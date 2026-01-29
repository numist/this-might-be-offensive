<div class="contentbox">
    <div class="blackbar"></div>
        <div class="heading">your stuff:</div>
        <div class="bluebox">
            <p>hi <b><?= me()->htmlUsername() ?></b>!</p>
            
            <p><a href="<?= Link::content("upload") ?>">upload</a></p>
            
            <p><a href="<?= Link::content("subscriptions") ?>">subscribed threads</a></p>
            
            <p><a href="<?= Link::content("settings") ?>">settings</a></p>

            <p><a href="logout.php">log out</a></p>
        </div>
    <div class="blackbar"></div>
</div>
