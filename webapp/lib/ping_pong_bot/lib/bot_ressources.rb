module BotRessources

  def self.kittens
    [
      'https://images.unsplash.com/photo-1445499348736-29b6cdfc03b9?w=800&q=80',
      'https://images.unsplash.com/photo-1503844281047-cf42eade5ca5?w=800&q=80',
      'https://images.unsplash.com/photo-1467839024528-ac3042ac0ae7?w=800&q=80',
      'https://images.unsplash.com/photo-1481134803835-48d6de104072?w=800&q=80',
      'https://images.unsplash.com/photo-1517172527855-d7a4feea491b?w=800&q=80'
    ]
  end

  def self.puppies
    [
      'https://images.unsplash.com/photo-1447684808650-354ae64db5b8?w=800&q=80',
      'https://images.unsplash.com/photo-1508946621775-9d59b75e074e?w=800&q=80',
      'https://images.unsplash.com/photo-1518914781460-a3ada465edec?w=800&q=80',
      'https://images.unsplash.com/photo-1456081445129-830eb8d4bfc6?w=800&q=80',
      'https://images.unsplash.com/photo-1521128591876-b4ace034003a?w=800&q=80'
    ]
  end

  def self.ducklings
    [
      'https://images.unsplash.com/photo-1442689859438-97407280183f?w=800&q=80',
      'https://images.unsplash.com/photo-1513039740139-de0804d85a20?w=800&q=80',
      'https://images.unsplash.com/photo-1516467790960-dfa408ed87be?w=800&q=80'
    ]
  end

  def self.intro(help = false)
    text = []

    if help
      intro_text = "<p>1. <strong>Help</strong></p>"
    else
      intro_text = "<p>1. <strong>Welcome!</strong></p>"
    end

    intro_text << <<-HTML
<p>I'm the <em>Ping Pong Bot</em>, i allow you to test the viky.ai chatbot system.</p>
<p>Start by entering the following command:</p>
<ul>
  <li><code>ping</code> or <code>pong</code> show simple bot response.</li>
  <li><code>image</code> show the image widget.</li>
  <li><code>video</code> show the video widget.</li>
</ul>
HTML

    text << intro_text

    text << <<-HTML
<p>2. <strong>Map widget</strong></p>
<p>Enter the following commands in order to play with maps (via Google Maps Embed API):</p>
<ul>
  <li><code>map-place</code> show map via <strong>Place mode</strong>.</li>
  <li><code>map-directions</code> show map via <strong>Directions mode</strong>.</li>
  <li><code>map-search</code> show map via <strong>Search mode</strong>.</li>
  <li><code>map-view</code> show map via <strong>View mode</strong>.</li>
  <li><code>map-streetview</code> show map via <strong>Street View mode</strong>.</li>
</ul>
HTML

    text << <<-HTML
<p>3. <strong>Button & Button group widget</strong></p>
<p>Enter the following commands in order to play with buttons:</p>
<ul>
  <li><code>button</code> show the button widget.</li>
  <li><code>button-group</code> show the button group widget.</li>
  <li><code>button-group-deactivable</code> show the button group widget with disable_on_click option enabled.</li>
</ul>
HTML

    text << <<-HTML
<p>4. <strong>Card & List widget</strong></p>
<ul>
  <li><code>card</code> show standard card widget.</li>
  <li><code>card-video</code> show card widget with video.</li>
  <li><code>hlist</code> show list with horizontal orientation.</li>
  <li><code>hlist-card</code> show list of cards with horizontal orientation.</li>
  <li><code>vlist</code> show list with vertical orientation.</li>
</ul>
HTML

    text << <<-HTML
<p>5. <strong>Change locale</strong></p>
<p>
  Use command <code>change-locale</code> in order to change speech to text locale.
  Speech to text functionality is only available under Chrome.
</p>
<p>Happy testing!</p>
HTML

    text
  end


end
